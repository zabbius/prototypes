#include "GunController.h"

#include <Arduino.h>


GunController::GunController( uint8_t powerPin, uint8_t firePin, uint8_t sensorPin, DisplayController& displayController )
    : PowerPin(powerPin), FirePin(firePin), SensorPin(sensorPin), _displayController(displayController)
{
    _fireIncrement = 8;
    _fireMaxPwm = 0x6F;
    
    _iconIndex = 0xFF;
    _powerValue = LOW;
    _fireValue = LOW;
}

void GunController::Init()
{
    _updateTick = millis();
    pinMode(PowerPin, OUTPUT);
    pinMode(FirePin, OUTPUT);
    pinMode(SensorPin, INPUT_PULLUP);
    
    digitalWrite(PowerPin, LOW);
    digitalWrite(FirePin, LOW);

    _iconId = _displayController.AllocateIcon();
    UpdateIcon();
}

void GunController::HandleCommand( Command& cmd, Print &output )
{
    static DisplayController::Alert gunEnabled("Gun enabled");
    static DisplayController::Alert gunDisabled("Gun disabled");
    static DisplayController::Alert gunFire("FIRE!!!");


    const char* action = "";
    
    cmd.GetStringParam("act", &action);
    
    if (strcmp(action, "power") == 0)
    {
        int value = 0;
        
        if (!cmd.GetIntParam("value", &value))
        {
            SendError(output, F("Value should be specified"));
            return;
        }
        
        if (value)
        {
            if (_powerValue == LOW)
                _displayController.PutAlert(&gunEnabled);
            _powerValue = HIGH;
        }
        else
        {
            if (_powerValue == HIGH)
                _displayController.PutAlert(&gunDisabled);
            
            _powerValue = LOW;
        }
        SendOk(output);
    }
    else if (strcmp(action, "fire") == 0)
    {
        if (_powerValue == HIGH)
        {
            if (_fireValue == LOW)
            {
                _displayController.PutAlert(&gunFire);
                _fireValue = HIGH;
                _finishFire = false;
            }
            SendOk(output);
        }
        else
            SendError(output, F("Gun power is off"));
    }
    else if (strcmp(action, "status") == 0)
        Status(cmd, output);
    else if (strcmp(action, "configure") == 0)
        Configure(cmd, output);
    else
        SendErrorFormat(output, "Invalid action: %s", action);
}

void GunController::Configure( Command& cmd, Print &output )
{
    if (!cmd.GetIntParam("fireIncrement", &_fireIncrement))
    {
        SendError(output, F("Invalid fireIncrement"));
        return;
    }
    
    if (!cmd.GetIntParam("fireMaxPwm", &_fireMaxPwm))
    {
        SendError(output, F("Invalid fireMaxPwm"));
        return;
    }
    
    _fireMaxPwm = constrain(_fireMaxPwm, 0x20, 0xFF);
    _fireIncrement = constrain(_fireIncrement, 4, 48);
    
    SendOk(output);
}

void GunController::Status( Command& cmd, Print &output )
{
    SendOkBegin(output);
    output.print(F(", \"power\": "));
    output.print((_powerValue || _fireValue) ? "true" : "false");
    output.print(F(", \"fire\": "));
    output.print(_fireValue ? "true" : "false");
    output.print(F(", \"fireIncrement\": "));
    output.print(_fireIncrement);
    output.print(F(", \"firemaxPwm\": "));
    output.print(_fireMaxPwm);
    SendOkEnd(output);
}

void GunController::UpdateIcon()
{
    static FlashIcon gunIcons[2] = 
    {
        PICON("\x00\x00\x00\x04\x00\x00\x00\x00"),
        PICON("\x0E\x04\x11\x1B\x11\x04\x0E\x00")
    };
    
    uint8_t iconIndex = _powerValue != LOW || _fireValue != LOW ? 1 : 0;
    
    if (iconIndex != _iconIndex)
    {
        _iconIndex = iconIndex;
        _displayController.ChangeIcon(_iconId, gunIcons[_iconIndex]);
    }
}

void GunController::Proceed()
{
    unsigned long tick = millis();
    
    if (tick - _updateTick >= UpdateInterval)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        if (_fireValue > LOW && digitalRead(SensorPin) == LOW)
            _finishFire = true;
        
        if (_fireValue > LOW)
        {
            _fireValue += _finishFire ? -_fireIncrement : _fireIncrement;
            _fireValue = constrain(_fireValue, LOW, _fireMaxPwm);
        }
        
        
        digitalWrite(PowerPin, _powerValue != LOW || _fireValue != LOW ? HIGH : LOW);
        analogWrite(FirePin, _fireValue);
        
        UpdateIcon();

        _updateTick = tick;
        digitalWrite(LED_BUILTIN, LOW);
    }
}
