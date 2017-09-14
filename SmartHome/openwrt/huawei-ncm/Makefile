#
# Copyright (C) 2015 Maarten Deprez <<A HREF="https://lists.openwrt.org/cgi-bin/mailman/listinfo/openwrt-devel">deprez.maarten at gmail.com</A>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=huawei-ncm
PKG_VERSION:=1
PKG_RELEASE:=0
PKG_LICENSE:=GPLv2

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/huawei-ncm
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=+kmod-usb-net-huawei-cdc-ncm
	TITLE:=Huawei NCM protocol
	PKGARCH:=all
	MAINTAINER:=Maarten Deprez deprez.maarten at gmail.com
endef

define Package/huawei-ncm/description
	Protocol scripts to make and maintain connections to 3G/4G networks over huawei NCM modems
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/huawei-ncm/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/lib/netifd/proto/huawei_ncm.sh $(1)/lib/netifd/proto/huawei_ncm.sh
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/usr/sbin/* $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/lib/huawei-ncm
	$(INSTALL_DATA) ./files/usr/lib/huawei-ncm/*.sh $(1)/usr/lib/huawei-ncm/
endef

$(eval $(call BuildPackage,huawei-ncm))
