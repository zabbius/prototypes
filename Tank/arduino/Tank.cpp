#include "Tank.h"
#include <Arduino.h>


Tank::Tank( int serialSpeed, uint8_t displayAddr, uint8_t battPin, uint8_t chargePin, uint8_t gunPowerPin, uint8_t gunFirePin, uint8_t gunSensorPin )
    : SerialSpeed(serialSpeed),
    _streamRequestHandler(Serial),
    _batteryController(battPin, chargePin, _displayController),
    _gunController(gunPowerPin, gunFirePin, gunSensorPin, _displayController),
    _messageController(_displayController),
    _iconController(_displayController),
    _displayController(displayAddr)
{
    _handlerInfo[0] = HandlerInfo("ping", &_pingHandler);
    _handlerInfo[1] = HandlerInfo("battery", &_batteryController);
    _handlerInfo[2] = HandlerInfo("gun", &_gunController);
    _handlerInfo[3] = HandlerInfo("display", &_messageController);
    _handlerInfo[4] = HandlerInfo("icon", &_iconController);
    _handlerInfo[5] = HandlerInfo("status", this);
    _streamRequestHandler.SetHandlers(_handlerInfo, 6);
}


void Tank::HandleCommand( Command& cmd, Print &output )
{
    Status(cmd, output);
}

void Tank::Status( Command& cmd, Print &output )
{
    output.println(F("{ \"status\": \"OK\""));
    output.print(F("\t, \"battery\": "));
    _batteryController.Status(cmd, output);
    output.print(F("\t, \"gun\": "));
    _gunController.Status(cmd, output);
    output.print(F("\t, \"display\": "));
    _messageController.Status(cmd, output);
    output.print(F("\t, \"icon\": "));
    _iconController.Status(cmd, output);
    output.println("}");
}


void Tank::Setup()
{
    Serial.begin(SerialSpeed);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    _displayController.Init();
    _batteryController.Init();
    _gunController.Init();
    _messageController.Init();
    _iconController.Init();
}


void Tank::Loop()
{
    _batteryController.Proceed();
    _gunController.Proceed();
    _messageController.Proceed();
    _iconController.Proceed();
    _streamRequestHandler.Proceed();
    _displayController.Proceed();
}