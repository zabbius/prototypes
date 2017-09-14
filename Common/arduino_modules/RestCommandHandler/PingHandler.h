#ifndef __PING_HANDLER_H__
#define __PING_HANDLER_H__


#include "CommandHandler.h"

class PingHandler : public CommandHandler
{
public:
    virtual void HandleCommand( Command& cmd, Print &output )
    {
        SendOk(output);
    }
};


#endif