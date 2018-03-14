#=============================================
# Project Config
#=============================================
NO_SDK_SSL = 1
NO_SDK_NETAPP = 1
NO_SDK_EXAMPLE = 1
#USE_AT = 1
#WIFI_API_SDK = 1
NEW_CONSOLE = 1
NEW_SNTP = 1

WEB_ADC_DRV = 1

include $(GCCMK_PATH)sdkset.mk
#=============================================
# Project Files
#=============================================
#user main
ADD_SRC_C += project/src/main.c
ADD_SRC_C += project/src/user_start.c
ADD_SRC_C += project/src/gcc_wrap.c
# components

ADD_SRC_C += project/src/console/console_api.c
ADD_SRC_C += project/src/console/atcmd_user.c
ADD_SRC_C += project/src/console/wifi_console.c
#ADD_SRC_C += project/src/console/spi_tst.c

# api
INCLUDES += project/inc/api
 
ADD_SRC_C += project/src/api/flash_eep.c
ADD_SRC_C += project/src/api/flash_utils.c

# wifi
ifndef WIFI_API_SDK
INCLUDES += project/wifi
INCLUDES += project/wifi/rtw_wpa_supplicant/src
INCLUDES += project/wifi/rtw_wowlan
INCLUDES += project/wifi/rtw_wpa_supplicant/wpa_supplicant
INCLUDES += project/wifi/wlan/realtek/include 
INCLUDES += project/wifi/wlan/realtek/src/osdep
INCLUDES += project/wifi/wlan/realtek/src/hci

SRC_C += project/wifi/wifi_api.c
SRC_C += project/wifi/wifi_api_scan.c
SRC_C += project/wifi/rtw_wpa_supplicant/wpa_supplicant/wifi_eap_config.c
SRC_C += project/wifi/rtw_wpa_supplicant/wpa_supplicant/wifi_wps_config.c
SRC_C += project/wifi/wifi_conf.c
SRC_C += project/wifi/wifi_ind.c
SRC_C += project/wifi/wifi_promisc.c
SRC_C += project/wifi/wifi_simple_config.c
SRC_C += project/wifi/wifi_util.c
endif
# net 
ADD_SRC_C += project/src/net/netbios/netbios.c
ADD_SRC_C += project/src/net/sntp/sntp.c  

# webfs
INCLUDES += project/inc/web
ADD_SRC_C += project/web/webfs.c
# web
ADD_SRC_C += project/web/tcp_srv_conn.c
ADD_SRC_C += project/web/web_srv.c
ADD_SRC_C += project/web/web_utils.c
ADD_SRC_C += project/web/web_websocket.c
ADD_SRC_C += project/web/websock.c
ADD_SRC_C += project/web/web_auth.c
ADD_SRC_C += project/web/web_int_callbacks.c
ADD_SRC_C += project/web/web_int_vars.c

ifdef WEB_ADC_DRV
CFLAGS += -DWEB_ADC_DRV=1
ADD_SRC_C += project/src/adc/adc_drv.c 
ADD_SRC_C += project/src/adc/adc_ws.c 
endif