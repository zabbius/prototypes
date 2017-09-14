#ifndef __REQUEST_HANDLER_H__
#define __REQUEST_HANDLER_H__

#include "CommandHandler.h"
#include <Print.h>

struct HandlerInfo
{
    const char* command;
    CommandHandler* handler;
    
    HandlerInfo(): command(0), handler(0) {}
    HandlerInfo(const char* c, CommandHandler* h): command(c), handler(h) {}
};

class RequestHandler
{
public:
    RequestHandler();
    
    void SetHandlers( HandlerInfo* handlers, size_t count );
    
    void HandleRequest( char* request, Print& output );
    
private:    
    HandlerInfo* _handlers;
    size_t _handlersCount;
};



#endif
