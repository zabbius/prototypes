#ifndef __ICON_CONTROLLER_H__
#define __ICON_CONTROLLER_H__

#include "CommandHandler.h"
#include "DisplayController.h"

class IconController : public CommandHandler
{
public:

    IconController(DisplayController& displayController );
    void Init();

    virtual void HandleCommand( Command& cmd, Print &output );
    void Proceed();
    void Status( Command& cmd, Print &output );

private:
    void Configure( Command& cmd, Print &output );

private:
    DisplayController& _displayController;
    
};


#endif
