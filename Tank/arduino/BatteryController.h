#ifndef __BATTERY_CONTROLLER_H__
#define __BATTERY_CONTROLLER_H__


#include "CommandHandler.h"
#include "DisplayController.h"

class BatteryController : public CommandHandler
{
public:
    static const unsigned UpdateInterval = 250;
    static const unsigned ChargeIconIntervalMultiplier = 4;

    const uint8_t BattPin;
    const uint8_t ChargePin;
    
    BatteryController( uint8_t battPin, uint8_t chargePin, DisplayController& displayController );
    void Init();
    
    virtual void HandleCommand( Command& cmd, Print &output );
    
    void Proceed();
    
    int GetBatteryPercent();

    inline int GetBatteryValue() { return _battValue; }
    inline bool IsCharging() { return _charging; }
    
    void Status( Command& cmd, Print &output );

private:
    void Configure( Command& cmd, Print &output );

    void UpdateIcon();
    
private:
    int _minValue;
    int _maxValue;
    
    int _battValue;
    bool _charging;
    
    uint8_t _chargingBlinkCount;
    
    DisplayController& _displayController;
    int8_t _iconId;
    
    unsigned long _updateTick;
};



#endif
