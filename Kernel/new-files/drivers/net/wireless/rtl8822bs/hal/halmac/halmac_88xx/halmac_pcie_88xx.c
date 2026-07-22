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

#include "halmac_pcie_88xx.h"

#if HALMAC_88XX_SUPPORT

/**
 * halmac_init_pcie_cfg_88xx() -  init PCIe
 * @pHalmac_adapter : the adapter of halmac
 * Author : KaiYuan Chang
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_init_pcie_cfg_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter
)
{
	VOID *pDriver_adapter = NULL;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	if (pHalmac_adapter->halmac_interface != HALMAC_INTERFACE_PCIE)
		return HALMAC_RET_WRONG_INTF;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_init_pcie_cfg_88xx ==========>\n");

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_init_pcie_cfg_88xx <==========\n");

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_deinit_pcie_cfg_88xx() - deinit PCIE
 * @pHalmac_adapter : the adapter of halmac
 * Author : KaiYuan Chang
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_deinit_pcie_cfg_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter
)
{
	VOID *pDriver_adapter = NULL;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	if (pHalmac_adapter->halmac_interface != HALMAC_INTERFACE_PCIE)
		return HALMAC_RET_WRONG_INTF;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_deinit_pcie_cfg_88xx ==========>\n");

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_deinit_pcie_cfg_88xx <==========\n");

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_cfg_rx_aggregation_88xx_pcie() - config rx aggregation
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_rx_agg_mode
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_cfg_rx_aggregation_88xx_pcie(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN PHALMAC_RXAGG_CFG phalmac_rxagg_cfg
)
{
	VOID *pDriver_adapter = NULL;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_cfg_rx_aggregation_88xx_pcie ==========>\n");

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_cfg_rx_aggregation_88xx_pcie <==========\n");

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_reg_read_8_pcie_88xx() - read 1byte register
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
u8
halmac_reg_read_8_pcie_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset
)
{
	VOID *pDriver_adapter = NULL;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;

	return PLATFORM_REG_READ_8(pDriver_adapter, halmac_offset);
}

/**
 * halmac_reg_write_8_pcie_88xx() - write 1byte register
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * @halmac_data : register value
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_reg_write_8_pcie_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset,
	IN u8 halmac_data
)
{
	VOID *pDriver_adapter = NULL;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;

	PLATFORM_REG_WRITE_8(pDriver_adapter, halmac_offset, halmac_data);

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_reg_read_16_pcie_88xx() - read 2byte register
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
u16
halmac_reg_read_16_pcie_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset
)
{
	VOID *pDriver_adapter = NULL;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;

	return PLATFORM_REG_READ_16(pDriver_adapter, halmac_offset);
}

/**
 * halmac_reg_write_16_pcie_88xx() - write 2byte register
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * @halmac_data : register value
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_reg_write_16_pcie_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset,
	IN u16 halmac_data
)
{
	VOID *pDriver_adapter = NULL;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;

	PLATFORM_REG_WRITE_16(pDriver_adapter, halmac_offset, halmac_data);

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_reg_read_32_pcie_88xx() - read 4byte register
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
u32
halmac_reg_read_32_pcie_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset
)
{
	VOID *pDriver_adapter = NULL;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;

	return PLATFORM_REG_READ_32(pDriver_adapter, halmac_offset);
}

/**
 * halmac_reg_write_32_pcie_88xx() - write 4byte register
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * @halmac_data : register value
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_reg_write_32_pcie_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset,
	IN u32 halmac_data
)
{
	VOID *pDriver_adapter = NULL;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;

	PLATFORM_REG_WRITE_32(pDriver_adapter, halmac_offset, halmac_data);

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_cfg_tx_agg_align_pcie_88xx() -config sdio bus tx agg alignment
 * @pHalmac_adapter : the adapter of halmac
 * @enable : function enable(1)/disable(0)
 * @align_size : sdio bus tx agg alignment size (2^n, n = 3~11)
 * Author : Soar Tu
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_cfg_tx_agg_align_pcie_88xx(
	IN PHALMAC_ADAPTER	pHalmac_adapter,
	IN u8	enable,
	IN u16	align_size
)
{
	return HALMAC_RET_NOT_SUPPORT;
}

/**
 * halmac_tx_allowed_pcie_88xx() - check tx status
 * @pHalmac_adapter : the adapter of halmac
 * @pHalmac_buf : tx packet, include txdesc
 * @halmac_size : tx packet size, include txdesc
 * Author : Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_tx_allowed_pcie_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 *pHalmac_buf,
	IN u32 halmac_size
)
{
	return HALMAC_RET_NOT_SUPPORT;
}

/**
 * halmac_reg_read_indirect_32_pcie_88xx() - read MAC reg by SDIO reg
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * Author : Soar
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
u32
halmac_reg_read_indirect_32_pcie_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset
)
{
	return 0xFFFFFFFF;
}

/**
 * halmac_reg_read_nbyte_pcie_88xx() - read n byte register
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * @halmac_size : register value size
 * @halmac_data : register value
 * Author : Soar
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_reg_read_nbyte_pcie_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset,
	IN u32 halmac_size,
	OUT u8 *halmac_data
)
{
	return HALMAC_RET_NOT_SUPPORT;
}

/**
 * halmac_set_bulkout_num_pcie_88xx() - inform bulk-out num
 * @pHalmac_adapter : the adapter of halmac
 * @bulkout_num : usb bulk-out number
 * Author : KaiYuan Chang
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_set_bulkout_num_pcie_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 bulkout_num
)
{
	return HALMAC_RET_NOT_SUPPORT;
}

/**
 * halmac_get_sdio_tx_addr_pcie_88xx() - get CMD53 addr for the TX packet
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_buf : tx packet, include txdesc
 * @halmac_size : tx packet size
 * @pcmd53_addr : cmd53 addr value
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_get_sdio_tx_addr_pcie_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 *halmac_buf,
	IN u32 halmac_size,
	OUT u32 *pcmd53_addr
)
{
	return HALMAC_RET_NOT_SUPPORT;
}

/**
 * halmac_get_usb_bulkout_id_pcie_88xx() - get bulk out id for the TX packet
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_buf : tx packet, include txdesc
 * @halmac_size : tx packet size
 * @bulkout_id : usb bulk-out id
 * Author : KaiYuan Chang
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_get_usb_bulkout_id_pcie_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 *halmac_buf,
	IN u32 halmac_size,
	OUT u8 *bulkout_id
)
{
	return HALMAC_RET_NOT_SUPPORT;
}

HALMAC_RET_STATUS
halmac_mdio_write_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 addr,
	IN u16 data,
	IN u8 speed
)
{
	u8 tmp_u1b = 0;
	u32 count = 0;
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;
	u8 real_addr = 0;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	HALMAC_REG_WRITE_16(pHalmac_adapter, REG_MDIO_V1, data);

	real_addr = (addr & 0x1F);
	HALMAC_REG_WRITE_8(pHalmac_adapter, REG_PCIE_MIX_CFG, real_addr);

	if (speed == HAL_INTF_PHY_PCIE_GEN1) {
		if (addr < 0x20)
			HALMAC_REG_WRITE_8(pHalmac_adapter, REG_PCIE_MIX_CFG + 3, 0x00);
		else
			HALMAC_REG_WRITE_8(pHalmac_adapter, REG_PCIE_MIX_CFG + 3, 0x01);
	} else if (speed == HAL_INTF_PHY_PCIE_GEN2) {
		if (addr < 0x20)
			HALMAC_REG_WRITE_8(pHalmac_adapter, REG_PCIE_MIX_CFG + 3, 0x02);
		else
			HALMAC_REG_WRITE_8(pHalmac_adapter, REG_PCIE_MIX_CFG + 3, 0x03);
	} else {
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_MDIO, HALMAC_DBG_ERR, "[ERR]Error Speed !\n");
	}

	HALMAC_REG_WRITE_8(pHalmac_adapter, REG_PCIE_MIX_CFG, HALMAC_REG_READ_8(pHalmac_adapter, REG_PCIE_MIX_CFG) | BIT_MDIO_WFLAG_V1);

	tmp_u1b = HALMAC_REG_READ_8(pHalmac_adapter, REG_PCIE_MIX_CFG) & BIT_MDIO_WFLAG_V1;
	count = 20;

	while (tmp_u1b && (count != 0)) {
		PLATFORM_RTL_DELAY_US(pDriver_adapter, 10);
		tmp_u1b = HALMAC_REG_READ_8(pHalmac_adapter, REG_PCIE_MIX_CFG) & BIT_MDIO_WFLAG_V1;
		count--;
	}

	if (tmp_u1b) {
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_MDIO, HALMAC_DBG_ERR, "[ERR]MDIO write fail!\n");
		return HALMAC_RET_FAIL;
	}

	return HALMAC_RET_SUCCESS;
}

u16
halmac_mdio_read_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 addr,
	IN u8 speed

)
{
	u16 ret = 0;
	u8 tmp_u1b = 0;
	u32 count = 0;
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;
	u8 real_addr = 0;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	real_addr = (addr & 0x1F);
	HALMAC_REG_WRITE_8(pHalmac_adapter, REG_PCIE_MIX_CFG, real_addr);

	if (speed == HAL_INTF_PHY_PCIE_GEN1) {
		if (addr < 0x20)
			HALMAC_REG_WRITE_8(pHalmac_adapter, REG_PCIE_MIX_CFG + 3, 0x00);
		else
			HALMAC_REG_WRITE_8(pHalmac_adapter, REG_PCIE_MIX_CFG + 3, 0x01);
	} else if (speed == HAL_INTF_PHY_PCIE_GEN2) {
		if (addr < 0x20)
			HALMAC_REG_WRITE_8(pHalmac_adapter, REG_PCIE_MIX_CFG + 3, 0x02);
		else
			HALMAC_REG_WRITE_8(pHalmac_adapter, REG_PCIE_MIX_CFG + 3, 0x03);
	} else {
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_MDIO, HALMAC_DBG_ERR, "[ERR]Error Speed !\n");
	}

	HALMAC_REG_WRITE_8(pHalmac_adapter, REG_PCIE_MIX_CFG, HALMAC_REG_READ_8(pHalmac_adapter, REG_PCIE_MIX_CFG) | BIT_MDIO_RFLAG_V1);

	tmp_u1b = HALMAC_REG_READ_8(pHalmac_adapter, REG_PCIE_MIX_CFG) & BIT_MDIO_RFLAG_V1;
	count = 20;

	while (tmp_u1b && (count != 0)) {
		PLATFORM_RTL_DELAY_US(pDriver_adapter, 10);
		tmp_u1b = HALMAC_REG_READ_8(pHalmac_adapter, REG_PCIE_MIX_CFG) & BIT_MDIO_RFLAG_V1;
		count--;
	}

	if (tmp_u1b) {
		ret  = 0xFFFF;
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_MDIO, HALMAC_DBG_ERR, "[ERR]MDIO read fail!\n");
	} else {
		ret = HALMAC_REG_READ_16(pHalmac_adapter, REG_MDIO_V1 + 2);
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_MDIO, HALMAC_DBG_TRACE, "[TRACE]Read Value = %x\n", ret);
	}

	return ret;
}

HALMAC_RET_STATUS
halmac_dbi_write32_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u16 addr,
	IN u32 data
)
{
	u8 tmp_u1b = 0;
	u32 count = 0;
	u16 write_addr = 0;
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	HALMAC_REG_WRITE_32(pHalmac_adapter, REG_DBI_WDATA_V1, data);

	write_addr = ((addr & 0x0ffc) | (0x000F << 12));
	HALMAC_REG_WRITE_16(pHalmac_adapter, REG_DBI_FLAG_V1, write_addr);

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_DBI, HALMAC_DBG_TRACE, "[TRACE]WriteAddr = %x\n", write_addr);

	HALMAC_REG_WRITE_8(pHalmac_adapter, REG_DBI_FLAG_V1 + 2, 0x01);
	tmp_u1b = HALMAC_REG_READ_8(pHalmac_adapter, REG_DBI_FLAG_V1 + 2);

	count = 20;
	while (tmp_u1b && (count != 0)) {
		PLATFORM_RTL_DELAY_US(pDriver_adapter, 10);
		tmp_u1b = HALMAC_REG_READ_8(pHalmac_adapter, REG_DBI_FLAG_V1 + 2);
		count--;
	}

	if (tmp_u1b) {
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_DBI, HALMAC_DBG_ERR, "[ERR]DBI write fail!\n");
		return HALMAC_RET_FAIL;
	}

	return HALMAC_RET_SUCCESS;
}

u32
halmac_dbi_read32_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u16 addr
)
{
	u16 read_addr = addr & 0x0ffc;
	u8 tmp_u1b = 0;
	u32 count = 0;
	u32 ret = 0;
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;


	HALMAC_REG_WRITE_16(pHalmac_adapter, REG_DBI_FLAG_V1, read_addr);

	HALMAC_REG_WRITE_8(pHalmac_adapter, REG_DBI_FLAG_V1 + 2, 0x2);
	tmp_u1b = HALMAC_REG_READ_8(pHalmac_adapter, REG_DBI_FLAG_V1 + 2);

	count = 20;
	while (tmp_u1b && (count != 0)) {
		PLATFORM_RTL_DELAY_US(pDriver_adapter, 10);
		tmp_u1b = HALMAC_REG_READ_8(pHalmac_adapter, REG_DBI_FLAG_V1 + 2);
		count--;
	}

	if (tmp_u1b) {
		ret  = 0xFFFF;
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_DBI, HALMAC_DBG_ERR, "[ERR]DBI read fail!\n");
	} else {
		ret = HALMAC_REG_READ_32(pHalmac_adapter, REG_DBI_RDATA_V1);
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_DBI, HALMAC_DBG_TRACE, "[TRACE]Read Value = %x\n", ret);
	}

	return ret;
}

HALMAC_RET_STATUS
halmac_dbi_write8_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u16 addr,
	IN u8 data
)
{
	u8 tmp_u1b = 0;
	u32 count = 0;
	u16 write_addr = 0;
	u16 remainder = addr & (4 - 1);
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	HALMAC_REG_WRITE_8(pHalmac_adapter, REG_DBI_WDATA_V1 + remainder, data);

	write_addr = ((addr & 0x0ffc) | (BIT(0) << (remainder + 12)));

	HALMAC_REG_WRITE_16(pHalmac_adapter, REG_DBI_FLAG_V1, write_addr);

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_DBI, HALMAC_DBG_TRACE, "[TRACE]WriteAddr = %x\n", write_addr);

	HALMAC_REG_WRITE_8(pHalmac_adapter, REG_DBI_FLAG_V1 + 2, 0x01);

	tmp_u1b = HALMAC_REG_READ_8(pHalmac_adapter, REG_DBI_FLAG_V1 + 2);

	count = 20;
	while (tmp_u1b && (count != 0)) {
		PLATFORM_RTL_DELAY_US(pDriver_adapter, 10);
		tmp_u1b = HALMAC_REG_READ_8(pHalmac_adapter, REG_DBI_FLAG_V1 + 2);
		count--;
	}

	if (tmp_u1b) {
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_DBI, HALMAC_DBG_ERR, "[ERR]DBI write fail!\n");
		return HALMAC_RET_FAIL;
	}

	return HALMAC_RET_SUCCESS;
}

u8
halmac_dbi_read8_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u16 addr
)
{
	u16 read_addr = addr & 0x0ffc;
	u8 tmp_u1b = 0;
	u32 count = 0;
	u8 ret = 0;
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	HALMAC_REG_WRITE_16(pHalmac_adapter, REG_DBI_FLAG_V1, read_addr);
	HALMAC_REG_WRITE_8(pHalmac_adapter, REG_DBI_FLAG_V1 + 2, 0x2);

	tmp_u1b = HALMAC_REG_READ_8(pHalmac_adapter, REG_DBI_FLAG_V1 + 2);

	count = 20;
	while (tmp_u1b && (count != 0)) {
		PLATFORM_RTL_DELAY_US(pDriver_adapter, 10);
		tmp_u1b = HALMAC_REG_READ_8(pHalmac_adapter, REG_DBI_FLAG_V1 + 2);
		count--;
	}

	if (tmp_u1b) {
		ret  = 0xFF;
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_DBI, HALMAC_DBG_ERR, "[ERR]DBI read fail!\n");
	} else {
		ret = HALMAC_REG_READ_8(pHalmac_adapter, REG_DBI_RDATA_V1 + (addr & (4 - 1)));
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_DBI, HALMAC_DBG_TRACE, "[TRACE]Read Value = %x\n", ret);
	}

	return ret;
}

HALMAC_RET_STATUS
halmac_trxdma_check_idle_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter
)
{
	u8 tmp_u8 = 0;
	u32 tmp_u32 = 0, count = 0;
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	/* Stop Tx & Rx DMA */
	HALMAC_REG_WRITE_32(pHalmac_adapter, REG_RXPKT_NUM, HALMAC_REG_READ_32(pHalmac_adapter, REG_RXPKT_NUM) | BIT(18));
	HALMAC_REG_WRITE_16(pHalmac_adapter, REG_PCIE_CTRL, HALMAC_REG_READ_16(pHalmac_adapter, REG_PCIE_CTRL) | ~(BIT(15)|BIT(8)));

	/* Stop FW */
	HALMAC_REG_WRITE_16(pHalmac_adapter, REG_SYS_FUNC_EN, HALMAC_REG_READ_16(pHalmac_adapter, REG_SYS_FUNC_EN) & ~(BIT(10)));

	/* Check Tx DMA is idle */
	count = 20;
	while ((HALMAC_REG_READ_8(pHalmac_adapter, REG_SYS_CFG5) & BIT(2)) == BIT(2)) {
		PLATFORM_RTL_DELAY_US(pDriver_adapter, 10);
		count--;
		if (count == 0) {
			PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_DBI, HALMAC_DBG_TRACE, "[ERR]PCIE Tx DMA check idle fail.\n");
			return HALMAC_RET_POWER_OFF_FAIL;
		}
	}

	/* Check Rx DMA is idle */
	count = 20;
	while ((HALMAC_REG_READ_32(pHalmac_adapter, REG_RXPKT_NUM) & BIT(17)) != BIT(17)) {
		PLATFORM_RTL_DELAY_US(pDriver_adapter, 10);
		count--;
		if (count == 0) {
			PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_DBI, HALMAC_DBG_TRACE, "[ERR]PCIE Rx DMA check idle fail.\n");
			return HALMAC_RET_POWER_OFF_FAIL;
		}
	}

	return HALMAC_RET_SUCCESS;
}
#endif /* HALMAC_88XX_SUPPORT */
