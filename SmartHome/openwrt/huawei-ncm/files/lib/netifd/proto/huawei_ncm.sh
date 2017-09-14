#!/bin/sh

. /lib/functions.sh
. ../netifd-proto.sh
init_proto "$@"

proto_huawei_ncm_init_config() {
    renew_handler=1

    proto_config_add_string 'pin'
    proto_config_add_string 'apn'
    proto_config_add_string 'mode'
    proto_config_add_string 'freq'
    proto_config_add_string 'timeout'
    proto_config_add_string 'interval'
}

proto_huawei_ncm_setup() {
    local config="$1"
    local iface="$2"

    local pin apn mode freq timeout interval
    json_get_vars pin apn mode freq timeout interval

    proto_export "INTERFACE=$config"
    proto_run_command "$config" /usr/sbin/huawei-ncm-connect -vvv \
        -p /var/run/huawei-ncm-$iface.pid \
        -s /lib/netifd/dhcp.script \
        -m "${mode:-2,0}" \
        -f "${freq:-0}" \
        -n "$pin" \
        -t "${timeout:-15}" \
        -i "${interval:-60}" \
        "$iface" \
        "$apn"

}

proto_huawei_ncm_renew() {
    local config="$1"
    proto_kill_command "$config" SIGHUP
}

proto_huawei_ncm_teardown() {
    local config="$1"
    proto_kill_command "$config"
}

add_protocol huawei_ncm
