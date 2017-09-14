#ifndef __COMMAND_HANDLER_H__
#define __COMMAND_HANDLER_H__

#include "Command.h"
#include <Print.h>

#include <stdarg.h>

class CommandHandler
{
public:
    virtual void HandleCommand( Command& cmd, Print &output );
    
    static void SendError( Print &output, const char* err );
    static void SendError( Print &output, const __FlashStringHelper *err);
    static void SendOk( Print &output );
    static void SendErrorFormat( Print &output, const char* fmt, ... );

    static void PrintJsonQuotedString( Print &output, const char* str );

protected:
    static void SendOkBegin(Print &output);
    static void SendOkEnd(Print &output);
    
private:
    static void SendErrorBegin(Print &output);
    static void SendErrorEnd(Print &output);
};




#endif
