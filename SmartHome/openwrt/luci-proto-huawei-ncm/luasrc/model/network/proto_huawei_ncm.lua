local proto = luci.model.network:register_protocol("huawei_ncm")

function proto.get_i18n(self)
    return luci.i18n.translate("Huawei NCM")
end

function proto.is_installed(self)
    return nixio.fs.access("/lib/netifd/proto/huawei_ncm.sh")
end

function proto.opkg_package(self)
    return "huawei_ncm"
end
