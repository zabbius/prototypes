#include "Command.h"
#include <string.h>
#include <stdlib.h>

Command::Command( char* str )
{
    _name = 0;
    _params = 0;
    _paramsLen = 0;
    
    if (str == 0 || *str != '/')
        return;
    
    _name = ++str;
    
    while (*str != 0)
    {
        if (*str == '?' && _params == 0)
        {
            _params = str + 1;
            _paramsLen = strlen(_params);
            *str = 0;
        }
        else if (*str == '&')
            *str = 0;
        ++str;
    }
}

const char* Command::GetParam( const char* name )
{
    const char *param = _params;
    size_t nameLen = strlen(name);
    
    size_t count = 0;
    
    while (count < _paramsLen)
    {
        size_t len = strlen(param);
        
        if (len != 0 && strncmp(param, name, nameLen) == 0)
        {
            const char* result = strchr(param, '=');
            
            if (result == 0)
                result = param + len;
            else
                ++result;
            
            return result;
        }
        count += len + 1;
        param += len + 1;
    }

    return 0;
}


bool Command::GetIntParam(  const char* paramName, int* out )
{
    const char* value = GetParam(paramName);
    
    if (value == 0)
        return false;
    
    *out = atoi(value);
    
    return true;
}
bool Command::GetDoubleParam(  const char* paramName, double* out )
{
    const char* value = GetParam(paramName);
    
    if (value == 0)
        return false;
    
    *out = atof(value);
    
    return true;
}

bool Command::GetStringParam(  const char* paramName, const char** out )
{
    const char* value = GetParam(paramName);
    
    if (value == 0)
        return false;
    
    *out = value;
    
    return true;
}
