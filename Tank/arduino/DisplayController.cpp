#include "DisplayController.h"

#include <string.h>
#include <Arduino.h>

DisplayController::DisplayController( uint8_t displayAddr )
    : _lcd(displayAddr, Cols, Rows)     
{
    _iconBusy = 0;
    _messageHead = _messageTail = 0;
    
    int n = MaxMessages;
    
    while (n--)
        _messages[n] = 0;
}

void DisplayController::Init()
{
    _lcd.init();
    _lcd.clear();
    _lcd.backlight();
    _messageTick = millis();
    _alertTick = millis();
}

int8_t DisplayController::AllocateIcon()
{
    for (int id = 0; id < MaxIcons; id++)
    {
        uint8_t bit = 1 << id;
        
        if ((_iconBusy & bit) == 0)
        {
            _iconBusy |= bit;
            UpdateIcons();
            return id;
        }
    }
    return -1;
}

void DisplayController::ReleaseIcon(int8_t iconId)
{
    uint8_t bit = 1 << iconId;
    
    _iconBusy &= ~bit;
    UpdateIcons();
}

void DisplayController::UpdateIcons()
{
    _lcd.setCursor(IconsCol, IconsRow);
    int spaces = MaxIcons;
     
    for (int id = 0; id < MaxIcons; id++)
    {
        uint8_t bit = 1 << id;
        
        if ((_iconBusy & bit) != 0)
        {
            _lcd.write(id);
            spaces--;
        }
    }
    
    while (spaces--)
        _lcd.write(' ');
}

void DisplayController::ChangeIcon(int8_t iconId, const uint8_t iconData[])
{
    uint8_t bit = 1 << iconId;
    
    if ((_iconBusy & bit) == 0)
        return;
    
    _lcd.createChar(iconId, iconData);
}

void DisplayController::ChangeIcon(int8_t iconId, FlashIcon iconData)
{
    uint8_t icon[8];
    memcpy_P(icon, iconData, 8);
    ChangeIcon(iconId, icon);
}


bool DisplayController::PutMessage( Message* pMessage )
{
    MessageInternal* &msg = _messages[_messageTail];
    
    if (_messageTail == _messageHead && msg != 0)
        return false;
    
    msg = pMessage;
    msg->position = -MessageLen;
    
    _messageTail++;
    if (_messageTail >= MaxMessages)
        _messageTail = 0;
    
    return true;
}

bool DisplayController::PutAlert( Alert* pAlert )
{
    AlertInternal* &alert = _alerts[_alertTail];
    
    if (_alertTail == _alertHead && alert != 0)
        return false;
    
    alert = pAlert;
    alert->done = false;
    alert->visible = false;
    
    _alertTail++;
    if (_alertTail >= MaxAlerts)
        _alertTail = 0;
    
    return true;
}

void DisplayController::PutSpecial( const char* text )
{
     _lcd.setCursor(SpecialAreaCol, SpecialAreaRow);
 
     int n = SpecialAreaSize;
     
     while (*text != 0 && n--)
        _lcd.write(*text);
}


void DisplayController::UpdateMessage()
{
    MessageInternal* &msg = _messages[_messageHead];
    
    _lcd.setCursor(MessageCol, MessageRow);
    
    uint8_t len = MessageLen;
    int n;
    
    for (n = msg->position ; n < 0 && len > 0; n++, len--)
        _lcd.write(' ');
    
    for (n = max(0, msg->position); n < msg->length && len > 0; n++, len--)
        _lcd.write(msg->text[n]);
    
    while (len--)
        _lcd.write(' ');
    
    msg->position++;
    
    if (msg->position > msg->length)
    {
        msg = 0;
        _messageHead++;

        if (_messageHead >= MaxMessages)
            _messageHead = 0;
    }
}

void DisplayController::UpdateAlert()
{
    AlertInternal* &alert = _alerts[_alertHead];
    
    _lcd.setCursor(AlertCol, AlertRow);
    
    uint8_t len = AlertLen;

    if (alert->visible)
    {   
        while (len--)
            _lcd.write(' ');

        alert->done = true;
        alert->visible = false;
        alert = 0;
        
        _alertHead++;

        if (_alertHead >= MaxAlerts)
            _alertHead = 0;
    }
    else
    {
        uint8_t n = (AlertLen - alert->length) / 2;
        
        while (n-- > 0)
        {
            len--;
            _lcd.write(' ');
        }
        
        for (n = 0; n < alert->length && len > 0; n++, len--)
            _lcd.write(alert->text[n]);
        
        while (len--)
            _lcd.write(' ');
        alert->visible = true;
    }
}

void DisplayController::Proceed()
{
    unsigned long tick = millis();
    
    if (HasAlerts())
    {
        if (tick - _alertTick >= AlertInterval)
        {
            digitalWrite(LED_BUILTIN, HIGH);
            UpdateAlert();
            _alertTick = tick;
            digitalWrite(LED_BUILTIN, LOW);
        }
    }
    else if (HasMessages())
    {
        if (tick - _messageTick >= MessageInterval)
        {
            digitalWrite(LED_BUILTIN, HIGH);
            UpdateMessage();
            _messageTick = tick;
            digitalWrite(LED_BUILTIN, LOW);
        }
    }
}
