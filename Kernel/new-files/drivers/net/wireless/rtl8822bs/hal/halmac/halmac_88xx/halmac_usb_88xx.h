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

#ifndef _HALMAC_USB_88XX_H_
#define _HALMAC_USB_88XX_H_

#include "../halmac_api.h"

#if HALMAC_88XX_SUPPORT

HALMAC_RET_STATUS
halmac_init_usb_cfg_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter
);

HALMAC_RET_STATUS
halmac_deinit_usb_cfg_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter
);


HALMAC_RET_STATUS
halmac_cfg_rx_aggregation_88xx_usb(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN PHALMAC_RXAGG_CFG phalmac_rxagg_cfg
);

u8
halmac_reg_read_8_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset
);

HALMAC_RET_STATUS
halmac_reg_write_8_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset,
	IN u8 halmac_data
);

u16
halmac_reg_read_16_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset
);

HALMAC_RET_STATUS
halmac_reg_write_16_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset,
	IN u16 halmac_data
);

u32
halmac_reg_read_32_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset
);

HALMAC_RET_STATUS
halmac_reg_write_32_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset,
	IN u32 halmac_data
);

HALMAC_RET_STATUS
halmac_set_bulkout_num_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 bulkout_num
);

HALMAC_RET_STATUS
halmac_get_usb_bulkout_id_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 *halmac_buf,
	IN u32 halmac_size,
	OUT u8 *bulkout_id
);

HALMAC_RET_STATUS
halmac_cfg_tx_agg_align_usb_88xx(
	IN PHALMAC_ADAPTER	pHalmac_adapter,
	IN u8	enable,
	IN u16	align_size
);

HALMAC_RET_STATUS
halmac_tx_allowed_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 *pHalmac_buf,
	IN u32 halmac_size
);

HALMAC_RET_STATUS
halmac_tx_allowed_pcie_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 *pHalmac_buf,
	IN u32 halmac_size
);

u32
halmac_reg_read_indirect_32_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset
);

HALMAC_RET_STATUS
halmac_reg_read_nbyte_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset,
	IN u32 halmac_size,
	OUT u8 *halmac_data
);

HALMAC_RET_STATUS
halmac_get_sdio_tx_addr_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 *halmac_buf,
	IN u32 halmac_size,
	OUT u32 *pcmd53_addr
);

HALMAC_RET_STATUS
halmac_set_usb_mode_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN HALMAC_USB_MODE usb_mode
);

HALMAC_RET_STATUS
halmac_usbphy_write_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 addr,
	IN u16 data,
	IN u8 speed
);

u16
halmac_usbphy_read_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 addr,
	IN u8 speed
);
#endif /* HALMAC_88XX_SUPPORT */

#endif/* _HALMAC_API_88XX_USB_H_ */
