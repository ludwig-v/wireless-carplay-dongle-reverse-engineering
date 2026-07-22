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

#ifndef _HALMAC_RX_BD_CHIP_H_
#define _HALMAC_RX_BD_CHIP_H_
#if (HALMAC_8814A_SUPPORT)

/*TXBD_DW0*/

#define GET_RX_BD_RXFAIL_8814A(__pRxBd)    GET_RX_BD_RXFAIL(__pRxBd)
#define GET_RX_BD_TOTALRXPKTSIZE_8814A(__pRxBd)    GET_RX_BD_TOTALRXPKTSIZE(__pRxBd)
#define GET_RX_BD_RXTAG_8814A(__pRxBd)    GET_RX_BD_RXTAG(__pRxBd)
#define GET_RX_BD_FS_8814A(__pRxBd)    GET_RX_BD_FS(__pRxBd)
#define GET_RX_BD_LS_8814A(__pRxBd)    GET_RX_BD_LS(__pRxBd)
#define GET_RX_BD_RXBUFFSIZE_8814A(__pRxBd)    GET_RX_BD_RXBUFFSIZE(__pRxBd)

/*TXBD_DW1*/

#define GET_RX_BD_PHYSICAL_ADDR_LOW_8814A(__pRxBd)    GET_RX_BD_PHYSICAL_ADDR_LOW(__pRxBd)

/*TXBD_DW2*/

#define GET_RX_BD_PHYSICAL_ADDR_HIGH_8814A(__pRxBd)    GET_RX_BD_PHYSICAL_ADDR_HIGH(__pRxBd)

#endif

#if (HALMAC_8822B_SUPPORT)

/*TXBD_DW0*/

#define GET_RX_BD_RXFAIL_8822B(__pRxBd)    GET_RX_BD_RXFAIL(__pRxBd)
#define GET_RX_BD_TOTALRXPKTSIZE_8822B(__pRxBd)    GET_RX_BD_TOTALRXPKTSIZE(__pRxBd)
#define GET_RX_BD_RXTAG_8822B(__pRxBd)    GET_RX_BD_RXTAG(__pRxBd)
#define GET_RX_BD_FS_8822B(__pRxBd)    GET_RX_BD_FS(__pRxBd)
#define GET_RX_BD_LS_8822B(__pRxBd)    GET_RX_BD_LS(__pRxBd)
#define GET_RX_BD_RXBUFFSIZE_8822B(__pRxBd)    GET_RX_BD_RXBUFFSIZE(__pRxBd)

/*TXBD_DW1*/

#define GET_RX_BD_PHYSICAL_ADDR_LOW_8822B(__pRxBd)    GET_RX_BD_PHYSICAL_ADDR_LOW(__pRxBd)

/*TXBD_DW2*/

#define GET_RX_BD_PHYSICAL_ADDR_HIGH_8822B(__pRxBd)    GET_RX_BD_PHYSICAL_ADDR_HIGH(__pRxBd)

#endif

#if (HALMAC_8197F_SUPPORT)

/*TXBD_DW0*/

#define GET_RX_BD_RXFAIL_8197F(__pRxBd)    GET_RX_BD_RXFAIL(__pRxBd)
#define GET_RX_BD_TOTALRXPKTSIZE_8197F(__pRxBd)    GET_RX_BD_TOTALRXPKTSIZE(__pRxBd)
#define GET_RX_BD_RXTAG_8197F(__pRxBd)    GET_RX_BD_RXTAG(__pRxBd)
#define GET_RX_BD_FS_8197F(__pRxBd)    GET_RX_BD_FS(__pRxBd)
#define GET_RX_BD_LS_8197F(__pRxBd)    GET_RX_BD_LS(__pRxBd)
#define GET_RX_BD_RXBUFFSIZE_8197F(__pRxBd)    GET_RX_BD_RXBUFFSIZE(__pRxBd)

/*TXBD_DW1*/

#define GET_RX_BD_PHYSICAL_ADDR_LOW_8197F(__pRxBd)    GET_RX_BD_PHYSICAL_ADDR_LOW(__pRxBd)

/*TXBD_DW2*/

#define GET_RX_BD_PHYSICAL_ADDR_HIGH_8197F(__pRxBd)    GET_RX_BD_PHYSICAL_ADDR_HIGH(__pRxBd)

#endif

#if (HALMAC_8821C_SUPPORT)

/*TXBD_DW0*/

#define GET_RX_BD_RXFAIL_8821C(__pRxBd)    GET_RX_BD_RXFAIL(__pRxBd)
#define GET_RX_BD_TOTALRXPKTSIZE_8821C(__pRxBd)    GET_RX_BD_TOTALRXPKTSIZE(__pRxBd)
#define GET_RX_BD_RXTAG_8821C(__pRxBd)    GET_RX_BD_RXTAG(__pRxBd)
#define GET_RX_BD_FS_8821C(__pRxBd)    GET_RX_BD_FS(__pRxBd)
#define GET_RX_BD_LS_8821C(__pRxBd)    GET_RX_BD_LS(__pRxBd)
#define GET_RX_BD_RXBUFFSIZE_8821C(__pRxBd)    GET_RX_BD_RXBUFFSIZE(__pRxBd)

/*TXBD_DW1*/

#define GET_RX_BD_PHYSICAL_ADDR_LOW_8821C(__pRxBd)    GET_RX_BD_PHYSICAL_ADDR_LOW(__pRxBd)

/*TXBD_DW2*/

#define GET_RX_BD_PHYSICAL_ADDR_HIGH_8821C(__pRxBd)    GET_RX_BD_PHYSICAL_ADDR_HIGH(__pRxBd)

#endif

#if (HALMAC_8198F_SUPPORT)

/*TXBD_DW0*/

#define GET_RX_BD_RXFAIL_8198F(__pRxBd)    GET_RX_BD_RXFAIL(__pRxBd)
#define GET_RX_BD_TOTALRXPKTSIZE_8198F(__pRxBd)    GET_RX_BD_TOTALRXPKTSIZE(__pRxBd)
#define GET_RX_BD_RXTAG_8198F(__pRxBd)    GET_RX_BD_RXTAG(__pRxBd)
#define GET_RX_BD_FS_8198F(__pRxBd)    GET_RX_BD_FS(__pRxBd)
#define GET_RX_BD_LS_8198F(__pRxBd)    GET_RX_BD_LS(__pRxBd)
#define GET_RX_BD_RXBUFFSIZE_8198F(__pRxBd)    GET_RX_BD_RXBUFFSIZE(__pRxBd)

/*TXBD_DW1*/

#define GET_RX_BD_PHYSICAL_ADDR_LOW_8198F(__pRxBd)    GET_RX_BD_PHYSICAL_ADDR_LOW(__pRxBd)

/*TXBD_DW2*/

#define GET_RX_BD_PHYSICAL_ADDR_HIGH_8198F(__pRxBd)    GET_RX_BD_PHYSICAL_ADDR_HIGH(__pRxBd)

#endif


#endif

