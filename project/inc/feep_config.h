/*
 * feep_config.h
 *
 *      Author: pvvx
 */

#ifndef _INC_FEEP_CONFIG_H_
#define _INC_FEEP_CONFIG_H_

//#define FEEP_ID_WIFI_CFG 0x4347 // id:'0W', type: struct wlan_fast_reconnect
//#define FEEP_ID_WIFI_AP_CFG 0x5041 // id:'1W', type: struct rtw_wifi_config_t

#define FEEP_ID_UART_CFG 0x5530 // id:'0U', type: UART_LOG_CONF
#define FEEP_ID_LWIP_CFG 0x4C30 // id:'0L', type: struct atcmd_lwip_conf
#define FEEP_ID_DHCP_CFG 0x4430 // id:'0D', type: struct _sdhcp_cfg


#endif /* _INC_FEEP_CONFIG_H_ */
