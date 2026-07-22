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

#include "halmac_pcie_8822b.h"
#include "halmac_pwr_seq_8822b.h"
#include "../halmac_init_88xx.h"
#include "../halmac_common_88xx.h"
#include "../halmac_pcie_88xx.h"
#include "../halmac_88xx_cfg.h"

#if HALMAC_8822B_SUPPORT

/**
 * halmac_mac_power_switch_8822b_pcie() - switch mac power
 * @pHalmac_adapter : the adapter of halmac
 * @halmac_power : power state
 * Author : KaiYuan Chang
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_mac_power_switch_8822b_pcie(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN HALMAC_MAC_POWER	halmac_power
)
{
	u8 interface_mask;
	u8 value8;
	u8 rpwm;
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;
	HALMAC_RET_STATUS status;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_PWR, HALMAC_DBG_TRACE, "[TRACE]halmac_mac_power_switch_88xx_pcie==========>\n", halmac_power);
	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_PWR, HALMAC_DBG_TRACE, "[TRACE]halmac_power = %x\n", halmac_power);
	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_PWR, HALMAC_DBG_TRACE, "[TRACE]8822B pwr seq ver = %s\n", HALMAC_8822B_PWR_SEQ_VER);

	interface_mask = HALMAC_PWR_INTF_PCI_MSK;

	pHalmac_adapter->rpwm_record = HALMAC_REG_READ_8(pHalmac_adapter, REG_PCIE_HRPWM1_V1);

	/* Check FW still exist or not */
	if (HALMAC_REG_READ_16(pHalmac_adapter, REG_MCUFW_CTRL) == 0xC078) {
		/* Leave 32K */
		rpwm = (u8)((pHalmac_adapter->rpwm_record ^ BIT(7)) & 0x80);
		HALMAC_REG_WRITE_8(pHalmac_adapter, REG_PCIE_HRPWM1_V1, rpwm);
	}

	value8 = HALMAC_REG_READ_8(pHalmac_adapter, REG_CR);
	if (value8 == 0xEA)
		pHalmac_adapter->halmac_state.mac_power = HALMAC_MAC_POWER_OFF;
	else
		pHalmac_adapter->halmac_state.mac_power = HALMAC_MAC_POWER_ON;

	/* Check if power switch is needed */
	if (halmac_power == HALMAC_MAC_POWER_ON && pHalmac_adapter->halmac_state.mac_power == HALMAC_MAC_POWER_ON) {
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_PWR, HALMAC_DBG_WARN, "[WARN]halmac_mac_power_switch power state unchange!\n");
		return HALMAC_RET_PWR_UNCHANGE;
	}

	if (halmac_power == HALMAC_MAC_POWER_OFF) {
		status = halmac_trxdma_check_idle_88xx(pHalmac_adapter);
		if (status != HALMAC_RET_SUCCESS)
			return status;
		if (halmac_pwr_seq_parser_88xx(pHalmac_adapter, HALMAC_PWR_CUT_ALL_MSK, HALMAC_PWR_FAB_TSMC_MSK,
			    interface_mask, halmac_8822b_card_disable_flow) != HALMAC_RET_SUCCESS) {
			PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_PWR, HALMAC_DBG_ERR, "[ERR]Handle power off cmd error\n");
			return HALMAC_RET_POWER_OFF_FAIL;
		}

		pHalmac_adapter->halmac_state.mac_power = HALMAC_MAC_POWER_OFF;
		pHalmac_adapter->halmac_state.ps_state = HALMAC_PS_STATE_UNDEFINE;
		pHalmac_adapter->halmac_state.dlfw_state = HALMAC_DLFW_NONE;
		halmac_init_adapter_dynamic_para_88xx(pHalmac_adapter);
	} else {
		if (halmac_pwr_seq_parser_88xx(pHalmac_adapter, HALMAC_PWR_CUT_ALL_MSK, HALMAC_PWR_FAB_TSMC_MSK,
			    interface_mask, halmac_8822b_card_enable_flow) != HALMAC_RET_SUCCESS) {
			PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_PWR, HALMAC_DBG_ERR, "[ERR]Handle power on cmd error\n");
			return HALMAC_RET_POWER_ON_FAIL;
		}

		pHalmac_adapter->halmac_state.mac_power = HALMAC_MAC_POWER_ON;
		pHalmac_adapter->halmac_state.ps_state = HALMAC_PS_STATE_ACT;
	}

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_PWR, HALMAC_DBG_TRACE, "[TRACE]halmac_mac_power_switch_88xx_pcie <==========\n");

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_pcie_switch_8822b() - pcie gen1/gen2 switch
 * @pHalmac_adapter : the adapter of halmac
 * @pcie_cfg : gen1/gen2 selection
 * Author : KaiYuan Chang
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_pcie_switch_8822b_pcie(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN HALMAC_PCIE_CFG	pcie_cfg
)
{
	VOID *pDriver_adapter = NULL;
	PHALMAC_API pHalmac_api;
	u8 current_link_speed = 0;
	u32 count = 0;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_PWR, HALMAC_DBG_TRACE, "[TRACE]halmac_pcie_switch_8822b ==========>\n");

	/**
	* Link Control 2 Register[3:0] Target Link Speed
	* Defined encodings are:
	* 0001b Target Link 2.5 GT/s
	* 0010b Target Link 5.0 GT/s
	* 0100b Target Link 8.0 GT/s
	*/

	if (pcie_cfg == HALMAC_PCIE_GEN1) {
		/* cfg 0xA0[3:0]=4'b0001 */
		halmac_dbi_write8_88xx(pHalmac_adapter, LINK_CTRL2_REG_OFFSET, (halmac_dbi_read8_88xx(pHalmac_adapter, LINK_CTRL2_REG_OFFSET) & 0xF0) | BIT(0));

		/* cfg 0x80C[17]=1 //PCIe DesignWave */
		halmac_dbi_write32_88xx(pHalmac_adapter, GEN2_CTRL_OFFSET, halmac_dbi_read32_88xx(pHalmac_adapter, GEN2_CTRL_OFFSET) | BIT(17));

		/* check link speed if GEN1 */
		/* cfg 0x82[3:0]=4'b0001 */
		current_link_speed = halmac_dbi_read8_88xx(pHalmac_adapter, LINK_STATUS_REG_OFFSET) & 0x0F;
		count = 2000;

		while ((current_link_speed != HALMAC_PCIE_GEN1_SPEED_88XX) && (count != 0)) {
			PLATFORM_RTL_DELAY_US(pDriver_adapter, 50);
			current_link_speed = halmac_dbi_read8_88xx(pHalmac_adapter, LINK_STATUS_REG_OFFSET) & 0x0F;
			count--;
		}

		if (current_link_speed != HALMAC_PCIE_GEN1_SPEED_88XX) {
			PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_PWR, HALMAC_DBG_ERR, "[ERR]Speed change to GEN1 fail !\n");
			return HALMAC_RET_FAIL;
		}

	} else if (pcie_cfg == HALMAC_PCIE_GEN2) {
		/* cfg 0xA0[3:0]=4'b0010 */
		halmac_dbi_write8_88xx(pHalmac_adapter, LINK_CTRL2_REG_OFFSET, (halmac_dbi_read8_88xx(pHalmac_adapter, LINK_CTRL2_REG_OFFSET) & 0xF0) | BIT(1));

		/* cfg 0x80C[17]=1 //PCIe DesignWave */
		halmac_dbi_write32_88xx(pHalmac_adapter, GEN2_CTRL_OFFSET, halmac_dbi_read32_88xx(pHalmac_adapter, GEN2_CTRL_OFFSET) | BIT(17));

		/* check link speed if GEN2 */
		/* cfg 0x82[3:0]=4'b0010 */
		current_link_speed = halmac_dbi_read8_88xx(pHalmac_adapter, LINK_STATUS_REG_OFFSET) & 0x0F;
		count = 2000;

		while ((current_link_speed != HALMAC_PCIE_GEN2_SPEED_88XX) && (count != 0)) {
			PLATFORM_RTL_DELAY_US(pDriver_adapter, 50);
			current_link_speed = halmac_dbi_read8_88xx(pHalmac_adapter, LINK_STATUS_REG_OFFSET) & 0x0F;
			count--;
		}

		if (current_link_speed != HALMAC_PCIE_GEN2_SPEED_88XX) {
			PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_PWR, HALMAC_DBG_ERR, "[ERR]Speed change to GEN1 fail !\n");
			return HALMAC_RET_FAIL;
		}

	} else {
		PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_PWR, HALMAC_DBG_ERR, "[ERR]Error Speed !\n");
		return HALMAC_RET_FAIL;
	}

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_PWR, HALMAC_DBG_TRACE, "[TRACE]halmac_pcie_switch_8822b <==========\n");

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_phy_cfg_8822b_pcie() - phy config
 * @pHalmac_adapter : the adapter of halmac
 * Author : KaiYuan Chang
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_phy_cfg_8822b_pcie(
	IN PHALMAC_ADAPTER pHalmac_adapter,
	IN HALMAC_INTF_PHY_PLATFORM platform
)
{
	VOID *pDriver_adapter = NULL;
	HALMAC_RET_STATUS status = HALMAC_RET_SUCCESS;
	PHALMAC_API pHalmac_api;

	if (halmac_adapter_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_ADAPTER_INVALID;

	if (halmac_api_validate(pHalmac_adapter) != HALMAC_RET_SUCCESS)
		return HALMAC_RET_API_INVALID;

	pDriver_adapter = pHalmac_adapter->pDriver_adapter;
	pHalmac_api = (PHALMAC_API)pHalmac_adapter->pHalmac_api;

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_PWR, HALMAC_DBG_TRACE, "[TRACE]halmac_phy_cfg ==========>\n");

	status = halmac_parse_intf_phy_88xx(pHalmac_adapter, HALMAC_RTL8822B_PCIE_PHY_GEN1, platform, HAL_INTF_PHY_PCIE_GEN1);

	if (status != HALMAC_RET_SUCCESS)
		return status;

	status = halmac_parse_intf_phy_88xx(pHalmac_adapter, HALMAC_RTL8822B_PCIE_PHY_GEN2, platform, HAL_INTF_PHY_PCIE_GEN2);

	if (status != HALMAC_RET_SUCCESS)
		return status;

	PLATFORM_MSG_PRINT(pDriver_adapter, HALMAC_MSG_PWR, HALMAC_DBG_TRACE, "[TRACE]halmac_phy_cfg <==========\n");

	return HALMAC_RET_SUCCESS;
}

/**
 * halmac_interface_integration_tuning_8822b_pcie() - pcie interface fine tuning
 * @pHalmac_adapter : the adapter of halmac
 * Author : Rick Liu
 * Return : HALMAC_RET_STATUS
 * More details of status code can be found in prototype document
 */
HALMAC_RET_STATUS
halmac_interface_integration_tuning_8822b_pcie(
	IN PHALMAC_ADAPTER pHalmac_adapter
)
{
	return HALMAC_RET_SUCCESS;
}

#endif /* HALMAC_8822B_SUPPORT*/
