obj-$(CONFIG_RTK_HCI_UART)	+= rtk_hci_uart.o
#obj-m	+= rtk_hci_uart.o
rtk_hci_uart-y	:= hci_ldisc.o rtk_coex.o
rtk_hci_uart-$(CONFIG_BT_HCIUART_RTKH4)  += hci_h4.o
rtk_hci_uart-$(CONFIG_BT_HCIUART_RTKH5)  += hci_rtk_h5.o
rtk_hci_uart-objs                        := $(rtk_hci_uart-y)
