### error function

VERBOSITY="${VERBOSITY:-1}"

error () {
        [ $VERBOSITY -ge 1 ] && echo "$@" >&2
        exit 1
}

vecho () {
        v=$1; shift
        [ $VERBOSITY -ge $v ] &&
        echo "$@" >&2
}
