#include "CommandHandler.h"


void CommandHandler::SendError( Print &output, const char* err )
{
    SendErrorBegin(output);
    output.print(err);
    SendErrorEnd(output);
}

void CommandHandler::SendError( Print &output, const __FlashStringHelper* err)
{
    SendErrorBegin(output);
    output.print(err);
    SendErrorEnd(output);
}

void CommandHandler::SendErrorFormat( Print &output, const char* fmt, ... )
{
    char buffer[256];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, 256, fmt, args);
    SendError(output, buffer);
    va_end (args);    
}

void CommandHandler::SendOk( Print &output )
{
    SendOkBegin(output);
    SendOkEnd(output);
}

void CommandHandler::PrintJsonQuotedString( Print &output, const char* str )
{
    if (!str)
    {
        output.print("null");
        return;
    }

    output.print('"');
    while (*str != 0)
    {
        if (*str == '"')
            output.print("\\\"");
        else
            output.print(*str);
            
        str++;
    }
    output.print('"');
}

void CommandHandler::SendOkBegin(Print &output)
{
    output.print(F("{ \"status\": \"OK\""));
}

void CommandHandler::SendOkEnd(Print &output)
{
    output.println(F(" }"));
}

void CommandHandler::SendErrorBegin(Print &output)
{
    output.print(F("{ \"status\": \"ERROR\", \"error\": \""));
}

void CommandHandler::SendErrorEnd(Print &output)
{
    output.println(F("\" }"));
}
