#include "IconController.h"
#include <Arduino.h>

IconController::IconController(DisplayController& displayController )
    : _displayController(displayController)
{
}

void IconController::Init()
{
}

void IconController::Proceed()
{
    //Nothing to do yet
}

void IconController::HandleCommand( Command& cmd, Print &output )
{
    const char* action = "";
    
    cmd.GetStringParam("act", &action);
    
    if (strcmp(action, "configure") == 0)
        Configure(cmd, output);
    else if (strcmp(action, "status") == 0)
        Status(cmd, output);
    else
        SendErrorFormat(output, "Invalid action: %s", action);
}

void IconController::Status( Command& cmd, Print &output )
{
    SendOk(output);
}


void IconController::Configure( Command& cmd, Print &output )
{
    SendOk(output);
}
