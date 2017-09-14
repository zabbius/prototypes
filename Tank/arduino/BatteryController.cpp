#include "BatteryController.h"
#include <Arduino.h>

#include <string.h>
#include <stdio.h>

BatteryController::BatteryController( uint8_t battPin, uint8_t chargePin, DisplayController& displayController )
    : BattPin(battPin), ChargePin(chargePin), _displayController(displayController)
{
    _charging = false;
    _chargingBlinkCount = 0;
    
    _minValue = 0;
    _maxValue = 1023;
    
    _battValue = 0;
}

void BatteryController::Init()
{
    _updateTick = millis();
    _iconId = _displayController.AllocateIcon();

    pinMode(ChargePin, INPUT);
    UpdateIcon();
}

void BatteryController::UpdateIcon()
{
    static const int BatteryPhaseCount = 6;
    
    static FlashIcon batteryIcons[BatteryPhaseCount] = 
    { 
        PICON("\x0E\x1F\x11\x11\x11\x11\x11\x1F"),
        PICON("\x0E\x1F\x11\x11\x11\x11\x1F\x1F"),
        PICON("\x0E\x1F\x11\x11\x11\x1F\x1F\x1F"),
        PICON("\x0E\x1F\x11\x11\x1F\x1F\x1F\x1F"),
        PICON("\x0E\x1F\x11\x1F\x1F\x1F\x1F\x1F"),
        PICON("\x0E\x1F\x1F\x1F\x1F\x1F\x1F\x1F"),
    };

    static FlashIcon chargeIcon = PICON("\x0A\x0A\x1F\x11\x11\x0E\x04\x04");

    if (_charging && ((_chargingBlinkCount / ChargeIconIntervalMultiplier) & 1) == 0)
    {
        _displayController.ChangeIcon(_iconId, chargeIcon);
    }
    else
    {
        int phase = map(_battValue, _minValue, _maxValue, 0, BatteryPhaseCount);
        phase = constrain(phase, 0, BatteryPhaseCount - 1);
        
        _displayController.ChangeIcon(_iconId, batteryIcons[phase]);
    }
}

void BatteryController::HandleCommand(Command& cmd, Print& output)
{
    const char* action = "";
    
    cmd.GetStringParam("act", &action);
    
    if (strcmp(action, "configure") == 0)
        Configure(cmd, output);
    else if (strcmp(action, "status") == 0)
        Status(cmd, output);
    else
        SendErrorFormat(output, "Invalid action: %s", action);
}

void BatteryController::Configure( Command& cmd, Print &output )
{
    if (!cmd.GetIntParam("minValue", &_minValue))
    {
        SendError(output, F("Invalid minValue"));
        return;
    }
    if (!cmd.GetIntParam("maxValue", &_maxValue))
    {
        SendError(output, F("Invalid maxValue"));
        return;
    }
    SendOk(output);
}

void BatteryController::Status( Command& cmd, Print &output )
{
    SendOkBegin(output);
    output.print(F(", \"charging\": "));
    output.print(_charging ? "true" : "false");
    output.print(F(", \"percent\": "));
    output.print(GetBatteryPercent());
    output.print(F(", \"value\": "));
    output.print(_battValue);
    output.print(F(", \"minValue\": "));
    output.print(_minValue);
    output.print(F(", \"maxValue\": "));
    output.print(_maxValue);
    SendOkEnd(output);
}

void BatteryController::Proceed()
{
    unsigned long tick = millis();
    
    if (tick - _updateTick >= UpdateInterval)
    {
        _chargingBlinkCount++;
        
        digitalWrite(LED_BUILTIN, HIGH);
        
        int newBattValue = analogRead(BattPin);
        bool newCharging = digitalRead(ChargePin) == HIGH;
        
        if (_charging != newCharging)
            _chargingBlinkCount = 0;

        _updateTick = tick;
        _battValue = newBattValue;
        _charging = newCharging;
        
        UpdateIcon();

        digitalWrite(LED_BUILTIN, LOW);
    }
}

int BatteryController::GetBatteryPercent()
{
    int percent = map(_battValue, _minValue, _maxValue, 0, 100);
    return constrain(percent, 0, 100);
}

