/*
 * web_auth.c
 *
 *  Created on: 23/04/2017.
 *      Author: pvvx
 */
#ifndef COMPILE_SCI  // Use Single Compilation Unit "web"
#include "autoconf.h"
#include "FreeRTOS.h"
#include "diag.h"
#include "web/web_utils.h"
#include "wifi_api.h"
#include "web/web_srv.h"
#endif	// COMPILE_SCI

/* ----------------------------------------------------------------------------------
 * pbuf[77] = Username and password are combined into a string "username:password"
 * Return: Authorization Level
 * 0 - Not Authorized  */

uint8_t UserAuthorization(uint8_t *pbuf, size_t declen)
{
		uint8_t * psw = strchr(pbuf, ':');
		if(psw != NULL) {
#if USE_WEB_AUTH_LEVEL
			if(strcmp(pbuf, "rtl871x:webfs_write") == 0) {
				return WEB_AUTH_LEVEL_WEBFS;
			}
			if(strcmp(pbuf, "rtl871x:ota_write") == 0) {
				return WEB_AUTH_LEVEL_OTA;
			}
			if(strcmp(pbuf, "rtl871x:supervisor") == 0) {
				return WEB_AUTH_LEVEL_SUPERVISOR;
			}
#endif
			*psw++ = 0;
			if(rom_xstrcmp(wifi_ap_cfg.ssid, pbuf)
			&& rom_xstrcmp( wifi_ap_cfg.password, psw)) {
				return WEB_AUTH_LEVEL_USER;
			}
			if(rom_xstrcmp(wifi_st_cfg.ssid, pbuf)
			&& rom_xstrcmp( wifi_st_cfg.password, psw)) {
				return WEB_AUTH_LEVEL_USER1;
			}
		}
		return 0;
}
