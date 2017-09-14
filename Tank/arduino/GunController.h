#ifndef __GUN_CONTROLLER_H__
#define __GUN_CONTROLLER_H__


#include "CommandHandler.h"
#include "DisplayController.h"

class GunController : public CommandHandler
{
public:
    static const unsigned UpdateInterval = 10;

    const uint8_t PowerPin;
    const uint8_t FirePin;
    const uint8_t SensorPin;

    GunController( uint8_t powerPin, uint8_t firePin, uint8_t sensorPin, DisplayController& displayController );
    void Init();
    
    virtual void HandleCommand( Command& cmd, Print &output );
    
    void Proceed();

    void Status( Command& cmd, Print &output );

private:
    void UpdateIcon();
    void Configure( Command& cmd, Print &output );

private:    
    DisplayController& _displayController;
    int8_t _iconId;
    
    uint8_t _iconIndex;

    uint8_t _powerValue;
    int     _fireValue;

    bool    _finishFire;
    
    int      _fireIncrement;
    int      _fireMaxPwm;

    unsigned long _updateTick;
};




#endif
