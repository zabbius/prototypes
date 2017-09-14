#ifndef __MESSAGE_CONTROLLER_H__
#define __MESSAGE_CONTROLLER_H__

#include "CommandHandler.h"
#include "DisplayController.h"

class MessageController : public CommandHandler
{
public:
    static const unsigned UpdateInterval = 500;
    static const unsigned MaxMessageLength = 255;
    
    MessageController( DisplayController& displayController );
    
    void Init();
    
    virtual void HandleCommand( Command& cmd, Print &output );
    void Proceed();
    void Status( Command& cmd, Print &output );

private:
    void Message( Command& cmd, Print &output );
    void Alert( Command& cmd, Print &output );

private:    
    DisplayController& _displayController;
//    unsigned long _updateTick;
    
    char _messageText[MaxMessageLength + 1];
    char _alertText[DisplayController::AlertLen + 1];
    
    DisplayController::Message  _message;
    DisplayController::Alert    _alert;
};

#endif