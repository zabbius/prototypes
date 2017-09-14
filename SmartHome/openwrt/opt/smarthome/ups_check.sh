#!/bin/sh

. /opt/smarthome/config



log()
{
    echo `date` "$1" >> /opt/smarthome/log/ups_check.log
}

timeout()
{
    log "Timeout reached, powerdown during $1 minutes"

    for num in $UPS_POWERDOWN_PHONE_LIST
    do
        log "Sending SMS to $num"
        /opt/smarthome/send_sms.sh "$num" "WARNING! Power is down during $1 minutes!"
    done
}

check_timeout()
{
    [[ "`find "$UPS_POWERDOWN_FLAG_FILE" -mmin +$1`" == "$UPS_POWERDOWN_FLAG_FILE" ]] || return 1

    log "Powerdown flag is older than $1 minutes"

    cat "$UPS_POWERDOWN_EXECUTED_HANDLERS" | grep -c "timeout_$1_called" > /dev/null && return 0

    log "Handler for timeout $1 was not called yet"

    timeout $1

    echo "timeout_$1_called" >> "$UPS_POWERDOWN_EXECUTED_HANDLERS"

    return 0
}

log "Checking ups power fail"

[[ -e "$UPS_POWERDOWN_FLAG_FILE" ]] || exit

log "Powerdown flag found"

check_timeout $UPS_POWERDOWN_TIMEOUT_1 || exit
check_timeout $UPS_POWERDOWN_TIMEOUT_2 || exit
check_timeout $UPS_POWERDOWN_TIMEOUT_3 || exit

