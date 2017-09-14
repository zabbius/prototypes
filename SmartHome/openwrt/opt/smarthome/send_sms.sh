#!/bin/sh

if [ -z "$2" ]
then
    echo "USAGE: $0 <number> <sms>"
    exit 1
fi


VERBOSITY=10

. /usr/lib/huawei-ncm/error.sh
. /usr/lib/huawei-ncm/find-modem.sh
. /usr/lib/huawei-ncm/modem.sh

find_modem_hard || exit
init_modem || exit

modem "+CMGF=1"
modem "+CMGS=\"$1\"\r\n$2\x1A"

