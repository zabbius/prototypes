#ifndef __PIN_CONTROLLER_H__
#define __PIN_CONTROLLER_H__

#include <inttypes.h>
#include "CommandHandler.h"

class PinController : public CommandHandler
{
public: 
    struct DigitalPin
    {
        uint8_t Number : 5;
        uint8_t Mode   : 2;
        uint8_t Value  : 1;
        const char* Name;

        bool IsValid() { return Name != 0; }
        
        DigitalPin()
            : Number(0), Mode(0), Value(0), Name(0)
            {}
        DigitalPin(uint8_t number, uint8_t mode, uint8_t value, const char* name)
            : Number(number), Mode(mode), Value(value), Name(name)
            {}
    };

public:
    static const uint8_t        MaxDigitalPins = 13;

    PinController();
    void SetPinDescription( DigitalPin pins[], uint8_t count );
    void Init();
    
    virtual void HandleCommand( Command& cmd, Print &output );
    void Proceed();
    void Status( Command& cmd, Print &output );

private:
    void DigitalWrite( Command& cmd, Print &output );
    void DigitalRead( Command& cmd, Print &output );
    void DigitalMode( Command& cmd, Print &output );

    DigitalPin* GetPin( Command& cmd, bool allowInvalid = false );
    void SendPinStatus(const DigitalPin& pin, Print& output);

    void SetPinDescription( const DigitalPin& pin );
    void InitPin( DigitalPin& pin );

private:
    DigitalPin _digitalPins[MaxDigitalPins + 1];
};


#endif 

