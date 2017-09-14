#include "StreamRequestHandler.h"
#include <Arduino.h>

StreamRequestHandler::StreamRequestHandler( Stream& stream, uint8_t statusPin )  
    : _stream(stream)
    , _statusPin(statusPin)
{
    _requestLen = 0;
}

void StreamRequestHandler::Proceed()
{
    while (_stream.available() > 0)
    {
        if (_statusPin != -1)
            digitalWrite(_statusPin, HIGH);
        int c = _stream.read();
        
        if (_requestLen >= BufferSize)
        {
            if (c == '\n' && _requestBuf[0] == '\r')
            {
                _stream.println(F("{ \"status\": \"ERROR\", \"error\": \"Request too long\" }"));
                _requestLen = 0;
            }
            else
                _requestBuf[0] = c;
            
            continue;
        }
        
        if (_requestLen > 0 && c == '\n' && _requestBuf[_requestLen - 1] == '\r')
        {
            _requestBuf[_requestLen - 1] = 0;
            
            HandleRequest(_requestBuf, _stream);
            _requestLen = 0;

            continue;
        }
        
        _requestBuf[_requestLen++] = c;
    }
    if (_statusPin != -1)
        digitalWrite(_statusPin, LOW);
}

    
