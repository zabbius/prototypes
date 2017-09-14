#!/bin/sh

. /opt/smarthome/config

action=`basename "$0"`

echo `date` "Action: $action" "Params: $*" >> /opt/smarthome/log/ups_action.log

case $action in
    "powerout")
        touch "$UPS_POWERDOWN_FLAG_FILE"
        ;;
    "mainsback")
        rm "$UPS_POWERDOWN_FLAG_FILE"
        rm "$UPS_POWERDOWN_EXECUTED_HANDLERS"
        ;;
esac

