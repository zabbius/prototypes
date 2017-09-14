#ifndef __SMARTHOME_H__
#define __SMARTHOME_H__

#include "RestCommandHandler.h"

#include "MessageController.h"
#include "IconController.h"
#include "DisplayController.h"
#include "PinController.h"

class SmartHome : public CommandHandler
{
public:
    const int SerialSpeed;

    SmartHome( int serialSpeed, uint8_t displayAddr );
    
    void Setup();
    void Loop();

    virtual void HandleCommand( Command& cmd, Print &output );

private:
    void Status( Command& cmd, Print &output );
    
private:
    StreamRequestHandler _streamRequestHandler;
    
    PingHandler _pingHandler;
    MessageController _messageController;
    IconController _iconController;
    PinController _pinController;

    HandlerInfo _handlerInfo[8];

    DisplayController _displayController;
};




#endif

