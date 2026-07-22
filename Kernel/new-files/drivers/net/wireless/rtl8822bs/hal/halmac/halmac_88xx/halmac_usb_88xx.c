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

#include "halmac_usb_88xx.h"

#if HALMAC_88XX_SUPPORT

/**
 * halmac_init_usb_cfg_88xx() - init USB
 * @pHalmac_adapter : the adapter of halmac
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_init_usb_cfg_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter
)
{
	VOID *pDriver_adapter = NULL;
	u8 value8 = 0;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_init_usb_cfg_88xx ==========>\n");

	value8 |= (BIT_DMA_MODE | (0x3 << BIT_SHIFT_BURST_CNT)); /* burst number = 4 */

	if (PLATFORM_REG_READ_8(pDriver_adapter, REG_SYS_CFG2 + 3) == 0x20) { /* usb3.0 */
		value8 |= (HALMAC_USB_BURST_SIZE_3_0 << BIT_SHIFT_BURST_SIZE);
	} else {
		if ((PLATFORM_REG_READ_8(pDriver_adapter, REG_USB_USBSTAT) & 0x3) == 0x1) /* usb2.0 */
			value8 |= HALMAC_USB_BURST_SIZE_2_0_HSPEED << BIT_SHIFT_BURST_SIZE;
		else /* usb1.1 */
			value8 |= HALMAC_USB_BURST_SIZE_2_0_FSPEED << BIT_SHIFT_BURST_SIZE;
	}

	PLATFORM_REG_WRITE_8(pDriver_adapter, REG_RXDMA_MODE, value8);
	PLATFORM_REG_WRITE_16(pDriver_adapter, REG_TXDMA_OFFSET_CHK, PLATFORM_REG_READ_16(pDriver_adapter, REG_TXDMA_OFFSET_CHK) | BIT_DROP_DATA_EN);

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_init_usb_cfg_88xx <==========\n");

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_deinit_usb_cfg_88xx() - deinit USB
 * @pHalmac_adapter : the adapter of halmac
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_deinit_usb_cfg_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter
)
{
	VOID *pDriver_adapter = NULL;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_deinit_usb_cfg_88xx ==========>\n");

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_deinit_usb_cfg_88xx <==========\n");

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_cfg_rx_aggregation_88xx_usb() - config rx aggregation
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_rx_agg_mode
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_cfg_rx_aggregation_88xx_usb(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN PHALMAC_RXAGG_CFG phalmac_rxagg_cfg
)
{
	u8 dma_usb_agg;
	u8 size = 0, timeout = 0, agg_enable = 0;
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_cfg_rx_aggregation_88xx_usb ==========>\n");

	dma_usb_agg = HALMAC_REG_READ_8(pHalmac_adapter, REG_RXDMA_AGG_PG_TH + 3);
	agg_enable = HALMAC_REG_READ_8(pHalmac_adapter, REG_TXDMA_PQ_MAP);

	switch (phalmac_rxagg_cfg->mode) {
	case HALMAC_RX_AGG_MODE_NONE:
		agg_enable &= ~BIT_RXDMA_AGG_EN;
		break;
	case HALMAC_RX_AGG_MODE_DMA:
		agg_enable |= BIT_RXDMA_AGG_EN;
		dma_usb_agg |= BIT(7);
		break;

	case HALMAC_RX_AGG_MODE_USB:
		agg_enable |= BIT_RXDMA_AGG_EN;
		dma_usb_agg &= ~BIT(7);
		break;
	default:
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_ERR, "[ERR]halmac_cfg_rx_aggregation_88xx_usb switch case not support\n");
		agg_enable &= ~BIT_RXDMA_AGG_EN;
		break;
	}

	if (phalmac_rxagg_cfg->threshold.drv_define == _FALSE) {
		if (PLATFORM_REG_READ_8(pDriver_adapter, REG_SYS_CFG2 + 3) == 0x20) {
			/* usb3.0 */
			size = 0x5;
			timeout = 0xA;
		} else {
			/* usb2.0 */
			size = 0x5;
			timeout = 0x20;
		}
	} else {
		size = phalmac_rxagg_cfg->threshold.size;
		timeout = phalmac_rxagg_cfg->threshold.timeout;
	}

	HALMAC_REG_WRITE_8(pHalmac_adapter, REG_TXDMA_PQ_MAP, agg_enable);
	HALMAC_REG_WRITE_8(pHalmac_adapter, REG_RXDMA_AGG_PG_TH + 3, dma_usb_agg);
	HALMAC_REG_WRITE_16(pHalmac_adapter, REG_RXDMA_AGG_PG_TH, (u16)(size | (timeout << BIT_SHIFT_DMA_AGG_TO_V1)));

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_cfg_rx_aggregation_88xx_usb <==========\n");

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_reg_read_8_usb_88xx() - read 1byte register
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
u8
halmac_reg_read_8_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset
)
{
	u8 value8;
	VOID *pDriver_adapter = NULL;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;

	/* PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_reg_read_8_usb_88xx ==========>\n"); */

	value8 = PLATFORM_REG_READ_8(pDriver_adapter, halmac_offset);

	/* PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_reg_read_8_usb_88xx <==========\n"); */

	return value8;
}

/**
 * halmac_reg_write_8_usb_88xx() - write 1byte register
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * @halmac_data : register value
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_reg_write_8_usb_88xx(
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

	/* PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_reg_write_8_usb_88xx ==========>\n"); */

	PLATFORM_REG_WRITE_8(pDriver_adapter, halmac_offset, halmac_data);

	/* PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_reg_write_8_usb_88xx <==========\n"); */

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_reg_read_16_usb_88xx() - read 2byte register
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
u16
halmac_reg_read_16_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset
)
{
	VOID *pDriver_adapter = NULL;

	union {
		u16	word;
		u8	byte[2];
	} value16 = { 0x0000 };

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;

	/* PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_reg_read_16_usb_88xx ==========>\n"); */

	value16.word = PLATFORM_REG_READ_16(pDriver_adapter, halmac_offset);

	/* PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_reg_read_16_usb_88xx <==========\n"); */

	return value16.word;
}

/**
 * halmac_reg_write_16_usb_88xx() - write 2byte register
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * @halmac_data : register value
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_reg_write_16_usb_88xx(
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

	/* PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_reg_write_16_usb_88xx ==========>\n"); */

	PLATFORM_REG_WRITE_16(pDriver_adapter, halmac_offset, halmac_data);

	/* PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_reg_write_16_usb_88xx <==========\n"); */

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_reg_read_32_usb_88xx() - read 4byte register
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
u32
halmac_reg_read_32_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset
)
{
	VOID *pDriver_adapter = NULL;

	union {
		u32	dword;
		u8	byte[4];
	} value32 = { 0x00000000 };

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;

	/* PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_reg_read_32_usb_88xx ==========>\n"); */

	value32.dword = PLATFORM_REG_READ_32(pDriver_adapter, halmac_offset);

	/* PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_reg_read_32_usb_88xx <==========\n"); */

	return value32.dword;
}

/**
 * halmac_reg_write_32_usb_88xx() - write 4byte register
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * @halmac_data : register value
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_reg_write_32_usb_88xx(
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

	/* PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_reg_write_32_usb_88xx ==========>\n"); */

	PLATFORM_REG_WRITE_32(pDriver_adapter, halmac_offset, halmac_data);

	/* PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_reg_write_32_usb_88xx <==========\n"); */

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_set_bulkout_num_usb_88xx() - inform bulk-out num
 * @pHalmac_adapter : the adapter of halmac
 * @bulkout_num : usb bulk-out number
 * Author : KaiYuan Chang
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_set_bulkout_num_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 bulkout_num
)
{
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	pHalmac_adapter->halmac_bulkout_num = bulkout_num;

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_get_usb_bulkout_id_usb_88xx() - get bulk out id for the TX packet
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_buf : tx packet, include txdesc
 * @halmac_size : tx packet size
 * @bulkout_id : usb bulk-out id
 * Author : KaiYuan Chang
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_get_usb_bulkout_id_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 *halmac_buf,
	IN u32 halmac_size,
	OUT u8 *bulkout_id
)
{
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;
	HALMAC_QUEUE_SELECT queue_sel;
	HALMAC_DMA_MAPPING dma_mapping;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_get_usb_bulkout_id_88xx ==========>\n");

	if (halmac_buf == NULL) {
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_ERR, "[ERR]halmac_buf is NULL!!\n");
		return HALMAC_RET_DATA_BUF_NULL;
	}

	if (halmac_size == 0) {
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_ERR, "[ERR]halmac_size is 0!!\n");
		return HALMAC_RET_DATA_SIZE_INCORRECT;
	}

	queue_sel = (HALMAC_QUEUE_SELECT)GET_TX_DESC_QSEL(halmac_buf);

	switch (queue_sel) {
	case HALMAC_QUEUE_SELECT_VO:
	case HALMAC_QUEUE_SELECT_VO_V2:
		dma_mapping = pHalmac_adapter->halmac_ptcl_queue[HALMAC_PTCL_QUEUE_VO];
		break;
	case HALMAC_QUEUE_SELECT_VI:
	case HALMAC_QUEUE_SELECT_VI_V2:
		dma_mapping = pHalmac_adapter->halmac_ptcl_queue[HALMAC_PTCL_QUEUE_VI];
		break;
	case HALMAC_QUEUE_SELECT_BE:
	case HALMAC_QUEUE_SELECT_BE_V2:
		dma_mapping = pHalmac_adapter->halmac_ptcl_queue[HALMAC_PTCL_QUEUE_BE];
		break;
	case HALMAC_QUEUE_SELECT_BK:
	case HALMAC_QUEUE_SELECT_BK_V2:
		dma_mapping = pHalmac_adapter->halmac_ptcl_queue[HALMAC_PTCL_QUEUE_BK];
		break;
	case HALMAC_QUEUE_SELECT_MGNT:
		dma_mapping = pHalmac_adapter->halmac_ptcl_queue[HALMAC_PTCL_QUEUE_MG];
		break;
	case HALMAC_QUEUE_SELECT_HIGH:
	case HALMAC_QUEUE_SELECT_BCN:
	case HALMAC_QUEUE_SELECT_CMD:
		dma_mapping = HALMAC_DMA_MAPPING_HIGH;
		break;
	default:
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_ERR, "[ERR]Qsel is out of range\n");
		return HALMAC_RET_QSEL_INCORRECT;
	}

	switch (dma_mapping) {
	case HALMAC_DMA_MAPPING_HIGH:
		*bulkout_id = 0;
		break;
	case HALMAC_DMA_MAPPING_NORMAL:
		*bulkout_id = 1;
		break;
	case HALMAC_DMA_MAPPING_LOW:
		*bulkout_id = 2;
		break;
	case HALMAC_DMA_MAPPING_EXTRA:
		*bulkout_id = 3;
		break;
	default:
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_ERR, "[ERR]DmaMapping is out of range\n");
		return HALMAC_RET_DMA_MAP_INCORRECT;
	}

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_INIT, HALMAC_DBG_TRACE, "[TRACE]halmac_get_usb_bulkout_id_88xx <==========\n");

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_cfg_tx_agg_align_usb_88xx() -config sdio bus tx agg alignment
 * @pHalmac_adapter : the adapter of halmac
 * @enable : function enable(1)/disable(0)
 * @align_size : sdio bus tx agg alignment size (2^n, n = 3~11)
 * Author : Soar Tu
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_cfg_tx_agg_align_usb_88xx(
	IN PHALMAC_ADAPTER	pHalmac_adapter,
	IN u8	enable,
	IN u16	align_size
)
{
	return HALMAC_RET_NOT_SUPPORT;
}

/**
 * halmac_tx_allowed_usb_88xx() - check tx status
 * @pHalmac_adapter : the adapter of halmac
 * @pHalmac_buf : tx packet, include txdesc
 * @halmac_size : tx packet size, include txdesc
 * Author : Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_tx_allowed_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 *pHalmac_buf,
	IN u32 halmac_size
)
{
	return HALMAC_RET_NOT_SUPPORT;
}

/**
 * halmac_reg_read_indirect_32_usb_88xx() - read MAC reg by SDIO reg
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * Author : Soar
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
u32
halmac_reg_read_indirect_32_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset
)
{
	return 0xFFFFFFFF;
}

/**
 * halmac_reg_read_nbyte_usb_88xx() - read n byte register
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_offset : register offset
 * @halmac_size : register value size
 * @halmac_data : register value
 * Author : Soar
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_reg_read_nbyte_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u32 halmac_offset,
	IN u32 halmac_size,
	OUT u8 *halmac_data
)
{
	return HALMAC_RET_NOT_SUPPORT;
}

/**
 * halmac_get_sdio_tx_addr_usb_88xx() - get CMD53 addr for the TX packet
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_buf : tx packet, include txdesc
 * @halmac_size : tx packet size
 * @pcmd53_addr : cmd53 addr value
 * Author : KaiYuan Chang/Ivan Lin
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_get_sdio_tx_addr_usb_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 *halmac_buf,
	IN u32 halmac_size,
	OUT u32 *pcmd53_addr
)
{
	return HALMAC_RET_NOT_SUPPORT;
}

HALMAC_RET_STATUS
halmac_set_usb_mode_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN HALMAC_USB_MODE usb_mode
)
{
	u32 usb_temp;
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;
	HALMAC_USB_MODE current_usb_mode;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	current_usb_mode = (HALMAC_REG_READ_8(pHalmac_adapter, REG_SYS_CFG2 + 3) == 0x20) ? HALMAC_USB_MODE_U3 : HALMAC_USB_MODE_U2;

	/*check if HW supports usb2_usb3 swtich*/
	usb_temp = HALMAC_REG_READ_32(pHalmac_adapter, REG_PAD_CTRL2);
	if (_FALSE == (BIT_GET_USB23_SW_MODE_V1(usb_temp) | (usb_temp & BIT_USB3_USB2_TRANSITION))) {
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_H2C, HALMAC_DBG_ERR, "[ERR]HALMAC_HW_USB_MODE usb mode HW unsupport\n");
		return HALMAC_RET_USB2_3_SWITCH_UNSUPPORT;
	}

	if (usb_mode == current_usb_mode) {
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_H2C, HALMAC_DBG_ERR, "[ERR]HALMAC_HW_USB_MODE usb mode unchange\n");
		return HALMAC_RET_USB_MODE_UNCHANGE;
	}

	usb_temp &= ~(BIT_USB23_SW_MODE_V1(0x3));

	if (usb_mode == HALMAC_USB_MODE_U2) {
		/* usb3 to usb2 */
		HALMAC_REG_WRITE_32(pHalmac_adapter, REG_PAD_CTRL2, usb_temp | BIT_USB23_SW_MODE_V1(HALMAC_USB_MODE_U2) | BIT_RSM_EN_V1);
	} else {
		/* usb2 to usb3 */
		HALMAC_REG_WRITE_32(pHalmac_adapter, REG_PAD_CTRL2, usb_temp | BIT_USB23_SW_MODE_V1(HALMAC_USB_MODE_U3) | BIT_RSM_EN_V1);
	}

	HALMAC_REG_WRITE_8(pHalmac_adapter, REG_PAD_CTRL2 + 1, 4); /* set counter down timer 4x64 ms */
	HALMAC_REG_WRITE_16(pHalmac_adapter, REG_SYS_PW_CTRL, HALMAC_REG_READ_16(pHalmac_adapter, REG_SYS_PW_CTRL) | BIT_APFM_OFFMAC);
	PLATFORM_RTL_DELAY_US(pDriver_adapter, 1000);
	HALMAC_REG_WRITE_32(pHalmac_adapter, REG_PAD_CTRL2, HALMAC_REG_READ_32(pHalmac_adapter, REG_PAD_CTRL2) | BIT_NO_PDN_CHIPOFF_V1);

	return HALMAC_RET_SUCCESS;
}

HALMAC_RET_STATUS
halmac_usbphy_write_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 addr,
	IN u16 data,
	IN u8 speed
)
{
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	if (speed == HAL_INTF_PHY_USB3) {
		HALMAC_REG_WRITE_8(pHalmac_adapter, 0xff0d, (u8)data);
		HALMAC_REG_WRITE_8(pHalmac_adapter, 0xff0e, (u8)(data >> 8));
		HALMAC_REG_WRITE_8(pHalmac_adapter, 0xff0c, addr | BIT(7));
	} else if (speed == HAL_INTF_PHY_USB2) {
		HALMAC_REG_WRITE_8(pHalmac_adapter, 0xfe41, (u8)data);
		HALMAC_REG_WRITE_8(pHalmac_adapter, 0xfe40, addr);
		HALMAC_REG_WRITE_8(pHalmac_adapter, 0xfe42, 0x81);
	} else {
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_USB, HALMAC_DBG_ERR, "[ERR]Error USB Speed !\n");
		return HALMAC_RET_NOT_SUPPORT;
	}

	return HALMAC_RET_SUCCESS;
}

u16
halmac_usbphy_read_88xx(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN u8 addr,
	IN u8 speed
)
{
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;
	u16 value = 0;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	if (speed == HAL_INTF_PHY_USB3) {
		HALMAC_REG_WRITE_8(pHalmac_adapter, 0xff0c, addr | BIT(6));
		value = (u16)(HALMAC_REG_READ_32(pHalmac_adapter, 0xff0c) >> 8);
	} else if (speed == HAL_INTF_PHY_USB2) {
		if ((addr >= 0xE0) && (addr <= 0xFF))
			addr -= 0x20;
		if ((addr >= 0xC0) && (addr <= 0xDF)) {
			HALMAC_REG_WRITE_8(pHalmac_adapter, 0xfe40, addr);
			HALMAC_REG_WRITE_8(pHalmac_adapter, 0xfe42, 0x81);
			value = HALMAC_REG_READ_8(pHalmac_adapter, 0xfe43);
		} else {
			PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_USB, HALMAC_DBG_ERR, "[ERR]Error USB2PHY offset!\n");
			return HALMAC_RET_NOT_SUPPORT;
		}
	} else {
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_USB, HALMAC_DBG_ERR, "[ERR]Error USB Speed !\n");
		return HALMAC_RET_NOT_SUPPORT;
	}

	return value;
}

#endif /* HALMAC_88XX_SUPPORT */
