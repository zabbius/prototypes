#include "PinController.h"
#include <Arduino.h>

PinController::PinController()
{
    for (uint8_t n = 0; n < MaxDigitalPins; n++)
        _digitalPins[n].Number = n;
}

void PinController::SetPinDescription( const PinController::DigitalPin& pin )
{
    uint8_t pinNumber = pin.Number;
    if (pinNumber < MaxDigitalPins)
        _digitalPins[pinNumber] = pin;
}

void PinController::SetPinDescription( PinController::DigitalPin pins[], uint8_t count )
{
    while (count--)
        SetPinDescription(pins[count]);
}

void PinController::InitPin( PinController::DigitalPin& pin )
{
    if (!pin.IsValid())
        return;

    pinMode(pin.Number, pin.Mode);
    if (pin.Mode == OUTPUT)
        digitalWrite(pin.Number, pin.Value);
    else if (pin.Mode == INPUT || pin.Mode == INPUT_PULLUP)
        pin.Value = (uint8_t)digitalRead(pin.Number);
}

void PinController::Init()
{
    for (uint8_t n = 0; n < MaxDigitalPins; n++)
        InitPin(_digitalPins[n]);
}

void PinController::Proceed()
{
    for (uint8_t n = 0; n < MaxDigitalPins; n++)
    {
        DigitalPin& pin = _digitalPins[n];
        if (pin.IsValid() && (pin.Mode == INPUT || pin.Mode == INPUT_PULLUP))
            pin.Value = (uint8_t) digitalRead(pin.Number);
    }
}

void PinController::HandleCommand( Command& cmd, Print &output )
{
    const char* action = "";
    
    cmd.GetStringParam("act", &action);
    
    if (strcmp(action, "dwrite") == 0)
        DigitalWrite(cmd, output);
    else if (strcmp(action, "dread") == 0)
        DigitalRead(cmd, output);
    else if (strcmp(action, "dmode") == 0)
        DigitalMode(cmd, output);
    else if (strcmp(action, "status") == 0)
        Status(cmd, output);
    else
        SendErrorFormat(output, "Invalid action: %s", action);
}

void PinController::SendPinStatus( const PinController::DigitalPin& pin, Print& output )
{
    output.print(F("{ \"number\": "));
    output.print(pin.Number);
    output.print(F(", \"mode\": "));
    switch (pin.Mode)
    {
    case INPUT:
        PrintJsonQuotedString(output, "input");
        break;
    case OUTPUT:
        PrintJsonQuotedString(output, "output");
        break;
    case INPUT_PULLUP:
        PrintJsonQuotedString(output, "pullup");
        break;
    default:
        break;
    }
    output.print(F(", \"value\": "));
    output.print(pin.Value);
    output.print(F(", \"name\": "));
    PrintJsonQuotedString(output, pin.Name);
    output.println(F(" }"));
}

void PinController::Status( Command& cmd, Print &output )
{
    SendOkBegin(output);
    output.print(F(", \"pins\": [ "));

    for (uint8_t n = 0; n < MaxDigitalPins; n++)
    {
        DigitalPin &pin = _digitalPins[n];
        if (pin.IsValid())
        {
            SendPinStatus(pin, output);
            output.print(F(", "));
        }
    }
    output.println(F(" ] }"));
}

PinController::DigitalPin* PinController::GetPin( Command& cmd, bool allowInvalid )
{
    const char* pinName = "";
    cmd.GetStringParam("pin", &pinName);

    int pinNumber = -1;

    if (pinName[0] == '!')
        pinNumber = atoi(pinName + 1);

    for (uint8_t n = 0; n < MaxDigitalPins; n++)
    {
        DigitalPin& pin = _digitalPins[n];

        if (allowInvalid || pin.IsValid())
        {
            if (pinNumber == pin.Number)
                return &pin;

            if (pin.Name && strcmp(pin.Name, pinName) == 0)
                return &pin;
        }
    }

    return 0;
}

void PinController::DigitalWrite( Command& cmd, Print &output )
{
    DigitalPin* pin = GetPin(cmd);

    if (pin == 0)
    {
        SendError(output, F("Invalid pin"));
        return;
    }
    
    if (pin->Mode != OUTPUT)
    {
        SendError(output, F("Invalid pin mode"));
        return;
    }
    
    int value = 0;
    cmd.GetIntParam("value", &value);
    pin->Value = (uint8_t)value;
    digitalWrite(pin->Number, pin->Value);
    SendOkBegin(output);
    output.print(F(", \"pin\": "));
    SendPinStatus(*pin, output);
    SendOkEnd(output);
}

void PinController::DigitalRead( Command& cmd, Print &output )
{
    DigitalPin* pin = GetPin(cmd);

    if (pin == 0)
    {
        SendError(output, F("Invalid pin"));
        return;
    }

    if (pin->Mode != INPUT_PULLUP && pin->Mode != INPUT)
    {
        SendError(output, F("Invalid pin mode"));
        return;
    }
    
    pin->Value = (uint8_t)digitalRead(pin->Number);
    SendOkBegin(output);
    output.print(F(", \"pin\": "));
    SendPinStatus(*pin, output);
    SendOkEnd(output);
}

void PinController::DigitalMode( Command& cmd, Print &output )
{
    DigitalPin *pin = GetPin(cmd, true);

    if (pin == 0)
    {
        SendError(output, F("Invalid pin"));
        return;
    }

    const char *mode = "";
    cmd.GetStringParam("mode", &mode);

    if (strcmp(mode, "input") == 0)
        pin->Mode = INPUT;
    else if (strcmp(mode, "output") == 0)
        pin->Mode = OUTPUT;
    else if (strcmp(mode, "pullup") == 0)
        pin->Mode = INPUT_PULLUP;
    else
    {
        SendError(output, F("Invalid mode"));
        return;
    }

    pin->Name = "";
    InitPin(*pin);
    SendOk(output);
}
