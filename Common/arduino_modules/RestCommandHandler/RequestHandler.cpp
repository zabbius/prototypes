#include "RequestHandler.h"
#include "Command.h"


RequestHandler::RequestHandler()
{
}

void RequestHandler::SetHandlers( HandlerInfo* handlers, size_t count )
{
    _handlers = handlers;
    _handlersCount = count;
}
 
void RequestHandler::HandleRequest( char* request, Print& output )
{
    Command cmd(request);
    
    for (size_t n = 0; n < _handlersCount; n++)
    {
        HandlerInfo& handleInfo = _handlers[n];
        if (strcmp(handleInfo.command, cmd.Name()) == 0)
        {
            handleInfo.handler->HandleCommand(cmd, output);
            return;
        }
    }
    
    output.print(F("{ \"status\": \"ERROR\", \"error\": \"Invalid command: '"));
    output.print(cmd.Name());
    output.println(F("'\" }"));
}
