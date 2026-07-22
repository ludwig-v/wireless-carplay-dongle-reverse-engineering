/******************************************************************************
 *
 * Copyright(c) 2016 - 2017 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 ******************************************************************************/

#ifndef _HALMAC_API_8822B_USB_H_
#define _HALMAC_API_8822B_USB_H_

#include "../../halmac_api.h"

#if HALMAC_8822B_SUPPORT

extern HALMAC_INTF_PHY_PARA HALMAC_RTL8822B_USB2_PHY[];
extern HALMAC_INTF_PHY_PARA HALMAC_RTL8822B_USB3_PHY[];

HALMAC_RET_STATUS
halmac_mac_power_switch_8822b_usb(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN HALMAC_MAC_POWER halmac_power
);

HALMAC_RET_STATUS
halmac_phy_cfg_8822b_usb(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN HALMAC_INTF_PHY_PLATFORM platform
);

HALMAC_RET_STATUS
halmac_pcie_switch_8822b_usb(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN HALMAC_PCIE_CFG	pcie_cfg
);

HALMAC_RET_STATUS
halmac_interface_integration_tuning_8822b_usb(
	IN PHALMAC_ADAPTER pHalmac_adapter
);

#endif /* HALMAC_8822B_SUPPORT*/

#endif/* _HALMAC_API_8822B_USB_H_ */
