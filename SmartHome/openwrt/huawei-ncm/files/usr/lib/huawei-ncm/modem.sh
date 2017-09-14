#include error.sh00

### open modem device

modem=

init_modem () {
    local m="${1:-$MODEM}"
    [ -c "$m" ] || error "modem $m does not exist"
    [ $VERBOSITY -ge 4 ] && echo "Opening modem $m" >&2
    exec 5<> "$m" || error "failed to open modem: $m"
    echo -en 'ATE0\r\n' >&5
    while read -rt1 -u5 i; do true; done
    modem="$m"
}


### send command and read response

modem() {

    [ -n "$modem" ] || error "modem not initialized"

    vecho 4 "flushing modem input" >&2
    while read -r -t1 -u5 i; do
        vecho 5 "modem: flush: $i" >&2
    done

    echo -en "AT$1\r\n" >&5

    if [ -n "$2" ]; then
        match="$2"
        select=
    else
        match="^$(echo "$1" | grep -o "^\W\w\+"):"
        select='s/^.*: *//'
    fi

    err=timeout; t0=$(($(date +%s)+5))
    while [ $((t=t0-$(date +%s))) -gt 0 ] && read -r -t$t -u5 i; do

        i="$(echo "$i" | sed 's/\r$//')"

        vecho 4 "modem: $1: $i" >&2

        #echo "$i" | grep -q "^AT" && continue

        if echo "$i" | grep -q '^OK'; then
            err=; break
        fi

        if echo "$i" | grep -q '^ERROR'; then
            err="command error"; break
        fi

        if echo "$i" | grep -q '^CME ERROR:'; then
            err="cme error ${i#*:}"; break
        fi

        if echo "$i" | grep -q "$match"; then
                        echo "$i" | sed "$select"
                        continue
                fi

        n=$((n+1))

    done

    if [ -n "$err" ]; then
        vecho 1 "modem error: $1: $err" >&2
        return 1
    fi

    return 0
}
