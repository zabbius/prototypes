--[[
LuCI - Lua Configuration Interface
]]--

local map, section, net = ...
local ifc = net:get_interface()

local apn, mode, freq, pin, timeout, interval
local bcast, defaultroute, peerdns, dns, metric


mode = section:taboption("general", ListValue, "mode",
    translate("Service mode"),
    translate("Allows to alter preferences  for or restrict to certain types of network"))
mode:value("gsm", translate("GSM only"))
mode:value("wcdma", translate("WCDMA only"))
mode:value("gsmfirst", translate("GSM, WCDMA"))
mode:value("wcdmafirst", translate("WCDMA, GSM"))
mode:value("auto", translate("Automatic"))
mode.default = "auto"


apn = section:taboption("general", Value, "apn", translate("APN"))

pin = section:taboption("general", Value, "pin", translate("PIN"))

timeout = section:taboption("advanced", Value, "timeout",
    translate("Registration and connection timeout"),
    translate("Time (in seconds) to wait for network registration and data connection establishment"))
timeout.placeholder = 15

interval = section:taboption("advanced", Value, "interval",
    translate("Connection check interval"),
    translate("Time (in seconds) between connection checks"))
interval.placeholder = 60

freq = section:taboption("advanced", Value, "freq",
    translate("Frequency lock"),
    translate("If set to a value other than none, lock to a given frequency (i.e. cell tower)"))

freq:value("0", translate("None"))
freq.default = "0"

local pipe = io.popen("/usr/bin/huawei-ncm-cells")
local line = pipe:read("*line")
while line do
    freq:value(line:match("%d+"), line)
    line = pipe:read("*line")
end

bcast = section:taboption("advanced", Flag, "broadcast",
        translate("Use broadcast flag"),
        translate("Required for certain ISPs, e.g. Charter with DOCSIS 3"))

bcast.default = bcast.disabled


defaultroute = section:taboption("advanced", Flag, "defaultroute",
    translate("Use default gateway"),
    translate("If unchecked, no default route is configured"))

defaultroute.default = defaultroute.enabled


peerdns = section:taboption("advanced", Flag, "peerdns",
    translate("Use DNS servers advertised by peer"),
    translate("If unchecked, the advertised DNS server addresses are ignored"))

peerdns.default = peerdns.enabled


dns = section:taboption("advanced", DynamicList, "dns",
    translate("Use custom DNS servers"))

dns:depends("peerdns", "")
dns.datatype = "ipaddr"
dns.cast     = "string"


metric = section:taboption("advanced", Value, "metric",
    translate("Use gateway metric"))

metric.placeholder = "0"
metric.datatype       = "uinteger"

luci.tools.proto.opt_macaddr(section, ifc, translate("Override MAC address"))


mtu = section:taboption("advanced", Value, "mtu", translate("Override MTU"))
mtu.placeholder = "1492"
mtu.datatype    = "max(9200)"
