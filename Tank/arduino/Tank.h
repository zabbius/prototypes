#ifndef __TANK_H__
#define __TANK_H__

#include "PingHandler.h"
#include "BatteryController.h"
#include "GunController.h"
#include "MessageController.h"
#include "IconController.h"
#include "StreamRequestHandler.h"
#include "DisplayController.h"

class Tank : public CommandHandler
{
public:
    const int SerialSpeed;

    Tank( int serialSpeed, uint8_t displayAddr, uint8_t battPin, uint8_t chargePin, uint8_t gunPowerPin, uint8_t gunFirePin, uint8_t gunSensorPin );
    
    void Setup();
    void Loop();

    virtual void HandleCommand( Command& cmd, Print &output );

private:
    void Status( Command& cmd, Print &output );
    
private:
    StreamRequestHandler _streamRequestHandler;
    
    PingHandler _pingHandler;
    BatteryController _batteryController;
    GunController _gunController;
    MessageController _messageController;
    IconController _iconController;
    
    HandlerInfo _handlerInfo[8];
    
    DisplayController _displayController;
};




#endif

