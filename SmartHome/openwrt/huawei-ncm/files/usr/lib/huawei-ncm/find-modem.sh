#include error.sh

### load module with checking

# load_module (module, [timeout])

load_module () {

    vecho 2 -n "Loading $1 module... "

    local t=0

    while ! [ -d "/sys/module/$1" ]; do

        if ! [ $((t++)) -lt ${2:-3} ]; then
            vecho 2 "timeout!" || vecho 1 "timeout loading $1 module"
            return 1
        fi

        if ! modprobe "$1"; then
            vecho 2 "failed!" || vecho 1 "unable to load $1 module"
            return 1
        fi

        sleep 1

    done

    vecho 2 "ok!"
    return 0

}


### find huawei modem device

# is_huawei_modem (iface)
is_huawei_modem () {
    [ "$(basename "$(readlink "/sys/class/net/$1/device/driver")")" = huawei_cdc_ncm ]
}

# find_modem_iface ([iface], [timeout])
find_modem_iface () {

    vecho 2 -n "Looking for huawei-cdc-ncm modem... "

    local i t=0

    while true; do

        for i in $([ -n "$1" ] && echo "$1" || ls /sys/class/net); do
            is_huawei_modem "$i" || continue
            IFACE="$i"; break 2
        done

        if ! [ $((t++)) -lt ${2:-5} ]; then
            vecho 2 "failed" || vecho 1 "no huawei-cdc-ncm modem found"
            return 1
        fi

        sleep 1

    done

    usbpath="$(readlink -f "/sys/class/net/$IFACE/device")"

    vecho 2 "ok!"
    return 0

}


### find interfaces
# use AT^SETPORT="FF;1,16,3,2" and restart
# to enable (only) necessary interfaces

check_interfaces () {
    [ -c "$MODEM" -a -c "$PCUI" -a -c "$WDM" ]
}

find_interfaces () {

    vecho 2 -n "Enumerating interfaces on $IFACE... "

    local i

    unset MODEM PCUI DIAG WDM

    for i in /sys/class/net/"$IFACE"/device/../*:*; do
        case $(cat "$i"/bInterfaceProtocol) in
            01) MODEM="/dev/$(basename "$i"/tty*)";;
            10) MODEM="/dev/$(basename "$i"/tty*)";;
            02) PCUI="/dev/$(basename "$i"/tty*)";;
            12) PCUI="/dev/$(basename "$i"/tty*)";;
            03) DIAG="/dev/$(basename "$i"/tty*)";;
            13) DIAG="/dev/$(basename "$i"/tty*)";;
            16) WDM="/dev/$(basename "$i/usbmisc"/*)";;
        esac
    done

    [ -c "$MODEM" ] || MODEM="$PCUI"

    # check if necessary interfaces are present

    if ! check_interfaces; then
        vecho 1 "missing interfaces from $IFACE:" \
            "$( [ -c "$MODEM" ] || echo modem )" \
            "$( [ -c "$PCUI" ] || echo pcui )" \
            "$( [ -c "$WDM" ] || echo wdm )"
        return 1
    fi

    vecho 2 "ok!"
    return 0

}


### usb connection path

cumpath () {

    local p i

    read p

    while read i; do
        p="$p/$i"
        echo "$i" | grep -qx "$1" &&
        echo "$p"
    done

}

usb_path () {
    echo "$1" | sed -e 's@\(^.*/usb[0-9]\+\|[^/]\+\)/@\1\<A HREF="https://lists.openwrt.org/cgi-bin/mailman/listinfo/openwrt-devel">n at g</A>' |
    cumpath '[0-9.-]\+'
}


### reset last usb hub in path

usbpath=

reset_usb () {

    [ -x /usr/bin/usbreset ] || return 1
    [ -n "$usbpath" ] || return 1

    local dev i

        vecho 2 -n "Trying to recover usb connectivity... "

    usb_path "$usbpath" | tac |

    while read i; do

        vecho 3 -n "($(basename "$i")) "

        [ -d "$i" ] || continue
        [ -f "$i/busnum" -a -f "$i/devnum" ] || continue

        dev=$(printf '%03d/%03d\n' $(cat "$i/busnum") $(cat "$i/devnum"))
        vecho 3 -n "--> [$dev] "

        usbreset $dev || break

        vecho 2 "ok!"
        return 0

    done

    vecho 2 "failed!"
    return 1

}


### reset modem device

reset_modem () {

    [ -c "$MODEM" ] || return 1

    vecho 2 -n "Resetting modem $IFACE... "

    echo -en "AT+CFUN=4\r\n" > "$MODEM"
    sleep 1
    echo -en "AT+CFUN=6,0\r\n" > "$MODEM"

    vecho 2 "ok!"
    return 0

}


### find modem

iface="$IFACE"

find_modem () {
    find_modem_iface "$iface" "${1:-10}" &&
    find_interfaces
}


### find modem with resilience

find_modem_hard () {

    local t=0

    load_module huawei_cdc_ncm || return 1

    while true; do

        find_modem ${1:-30} && return 0

        [ $((t++)) -lt 2 ] || return 1

        reset_modem ||
        #reset_usb ||
        return 1

    done

}
