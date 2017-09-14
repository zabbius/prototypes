#ifndef __DISPLAY_CONTROLLER_H__
#define __DISPLAY_CONTROLLER_H__

#include <inttypes.h>
#include <string.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

#include <avr/pgmspace.h>

class __FlashIconHelper;
typedef const __FlashIconHelper* FlashIcon;
#define PICON(data) (reinterpret_cast<const __FlashIconHelper *>(PSTR(data)))

class DisplayController
{
public:
    static const uint8_t        MaxIcons = 8;
    static const uint8_t        Rows = 2;
    static const uint8_t        Cols = 16;
    
    static const uint8_t        IconsRow = 0;
    static const uint8_t        IconsCol = 0;
    
    static const uint8_t        SpecialAreaRow  = 0;
    static const uint8_t        SpecialAreaCol  = 8;
    static const uint8_t        SpecialAreaSize = 8;
    
    static const uint8_t        MaxMessages = 8;
    static const unsigned       MessageInterval = 250;
    static const uint8_t        MessageRow = 1;
    static const uint8_t        MessageCol = 0;
    static const uint8_t        MessageLen = 16;

    static const uint8_t        MaxAlerts = 4;
    static const unsigned       AlertInterval = 1000;
    static const uint8_t        AlertRow = 1;
    static const uint8_t        AlertCol = 0;
    static const uint8_t        AlertLen = 16;

private:
    struct MessageInternal
    {
        const char*     text;
        uint8_t         length;
        int8_t          position;

        inline MessageInternal( const char* messageText )
            : text(messageText), length(strlen(messageText)), position(-MessageLen)
        {
        }
    };
    
    struct AlertInternal
    {
        const char*     text;
        uint8_t         length;
        bool            visible;
        bool            done;

        inline AlertInternal( const char* alertText ) 
            : text(alertText), length(min(strlen(text), AlertLen)), visible(false), done(false)
        {
        }
    };        

public:
    struct Message : public MessageInternal
    {
        inline Message( const char* messageText ) : MessageInternal(messageText) {}
        inline const char* GetText() { return text; }
        inline uint8_t GetLength() { return length; }
        inline bool IsDone() { return position > length; }
    };

    struct Alert : public AlertInternal
    {
        inline Alert( const char* alertText ) : AlertInternal(alertText) {}
        inline const char* GetText() { return text; }
        inline uint8_t GetLength() { return length; }
        inline bool IsDone() { return done; }
    };

public:
    DisplayController( uint8_t displayAddr );

    void Init();
    
    int8_t AllocateIcon();
    void ReleaseIcon(int8_t iconId);
    
    void ChangeIcon(int8_t iconId, const uint8_t iconData[]);
    void ChangeIcon(int8_t iconId, FlashIcon iconData);
    
    void Proceed();
    
    bool PutMessage( Message* pMessage );
    bool PutAlert( Alert* pAlert );
    void PutSpecial( const char* text );
    
    inline bool HasMessages() { return _messages[_messageHead] != 0; }
    inline bool HasAlerts() { return _alerts[_alertHead] != 0; }
    
private:    
    void UpdateIcons();
    void UpdateMessage();
    void UpdateAlert();

private:    
    LiquidCrystal_I2C   _lcd;
    
    uint8_t             _iconBusy;

    unsigned long       _messageTick;
    MessageInternal*    _messages[MaxMessages];
    uint8_t             _messageHead;
    uint8_t             _messageTail;

    unsigned long       _alertTick;
    AlertInternal*      _alerts[MaxAlerts];
    uint8_t             _alertHead;
    uint8_t             _alertTail;
};




#endif
