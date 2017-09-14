module("luci.controller.huawei_ncm", package.seeall)

function index()
    local page

    page = entry({"admin", "status", "huawei_ncm"}, template("huawei_ncm/status"), _("Huawei NCM"))
    page.dependent = true
end
