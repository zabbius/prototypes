#ifndef __STREAM_REQUEST_HANDLER_H__
#define __STREAM_REQUEST_HANDLER_H__

#include "RequestHandler.h"
#include <Stream.h>

class StreamRequestHandler : public RequestHandler
{
public:
    static const size_t BufferSize = 128;
    
    StreamRequestHandler( Stream& stream, uint8_t statusPin );
    void Proceed();

private:
    uint8_t     _statusPin;
    Stream&     _stream;
    char        _requestBuf[BufferSize];
    size_t      _requestLen;
};

#endif
