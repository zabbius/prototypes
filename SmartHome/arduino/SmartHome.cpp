#include "SmartHome.h"
#include <Arduino.h>
#include <HardwareSerial.h>


SmartHome::SmartHome( int serialSpeed, uint8_t displayAddr )
    : SerialSpeed(serialSpeed),
    _streamRequestHandler(Serial, LED_BUILTIN),
    _messageController(_displayController),
    _iconController(_displayController),
    _displayController(displayAddr),
    _pinController()
{
    _handlerInfo[0] = HandlerInfo("ping", &_pingHandler);
    _handlerInfo[1] = HandlerInfo("display", &_messageController);
    _handlerInfo[2] = HandlerInfo("icon", &_iconController);
    _handlerInfo[3] = HandlerInfo("pin", &_pinController);
    _handlerInfo[4] = HandlerInfo("status", this);
    _streamRequestHandler.SetHandlers(_handlerInfo, 6);
}


void SmartHome::HandleCommand( Command& cmd, Print &output )
{
    Status(cmd, output);
}

void SmartHome::Status( Command& cmd, Print &output )
{
    SendOkBegin(output);
    output.print(F("\t, \"display\": "));
    _messageController.Status(cmd, output);
    output.print(F("\t, \"icon\": "));
    _iconController.Status(cmd, output);
    output.print(F("\t, \"pin\": "));
    _pinController.Status(cmd, output);
    SendOkEnd(output);
}


void SmartHome::Setup()
{
    Serial.begin(SerialSpeed);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    _pinController.Init();
    _displayController.Init();
    _messageController.Init();
    _iconController.Init();
}

void SmartHome::Loop()
{
    _messageController.Proceed();
    _pinController.Proceed();
    _iconController.Proceed();
    _streamRequestHandler.Proceed();
    _displayController.Proceed();
}
