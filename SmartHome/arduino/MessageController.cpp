#include "MessageController.h"

MessageController::MessageController(DisplayController& displayController)
    : _displayController(displayController), _message("Arduino module started"), _alert("READY")
{
}

void MessageController::Init()
{
    _displayController.PutAlert(&_alert);
    _displayController.PutMessage(&_message);
}

void MessageController::Proceed()
{
}

void MessageController::Alert(Command& cmd, Print& output)
{
    if (!_alert.IsDone())
        SendError(output, F("Previous alert is not done yet"));

    const char* text;
    
    if (!cmd.GetStringParam("text", &text))
    {
        SendError(output, F("Text should be specified"));
        return;
    }
    
    strncpy(_alertText, text, DisplayController::AlertLen);

    _alert = DisplayController::Alert(_alertText);
    _displayController.PutAlert(&_alert);
    SendOk(output);
}

void MessageController::Message(Command& cmd, Print& output)
{
    if (!_message.IsDone())
        SendError(output, F("Previous message is not done yet"));

    const char* text;
    
    if (!cmd.GetStringParam("text", &text))
    {
        SendError(output, F("Text should be specified"));
        return;
    }
    
    strncpy(_messageText, text, MaxMessageLength);

    _message = DisplayController::Message(_messageText);
    _displayController.PutMessage(&_message);
    SendOk(output);
}

void MessageController::Status(Command& cmd, Print& output)
{
    SendOkBegin(output);
    output.print(F(", \"message\": { \"text\": "));
    PrintJsonQuotedString(output, _message.GetText());
    output.print(F(", \"done \": "));
    output.print(_message.IsDone() ? "true" : "false");
    output.print(F(" }, \"alert\": { \"text\": "));
    PrintJsonQuotedString(output, _alert.GetText());
    output.print(F(", \"done \": "));
    output.print(_message.IsDone() ? "true" : "false");
    output.println(F(" } }"));
}


void MessageController::HandleCommand( Command& cmd, Print &output )
{
    const char* action = "";
    cmd.GetStringParam("act", &action);
    
    if (strcmp(action, "message") == 0)
        Message(cmd, output);
    else if (strcmp(action, "alert") == 0)
        Alert(cmd, output);
    else if (strcmp(action, "status") == 0)
        Status(cmd, output);
    else
        SendErrorFormat(output, "Invalid action: %s", action);
}

