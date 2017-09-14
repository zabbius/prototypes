#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <string.h>

class Command
{
public:
    Command( char* str );
    
    inline const char* Name() const { return _name; }
    inline const char* Params() const { return _params; }
    
    const char* GetParam( const char* paramName );
    
    bool GetIntParam(  const char* paramName, int* out );
    bool GetDoubleParam(  const char* paramName, double* out );
    bool GetStringParam(  const char* paramName, const char** out );
    
private:
    char*  _name;
    char*  _params;
    size_t _paramsLen;
};

#endif