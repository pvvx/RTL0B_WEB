/*
 *  WebSclLib: Single Compilation Unit "web"
 */

#define COMPILE_SCI 1

#include "user_config.h"
#ifdef USE_WEB
#include "autoconf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "hal_crypto.h"
#include "rtl8711b_crypto.h"
#include "lwip/tcp.h"
#include "web/tcp_srv_conn.h"
#include "web/web_srv_int.h"
#include "web/web_utils.h"
#include "flash_eep.h"
#include "device_lock.h"
#include "web/webfs.h"
#include "web/sys_cfg.h"
#include "wifi_api.h"
#include "sys_api.h"

#include "web_srv.h"

#ifdef WEBSOCKET_ENA
#include "web/web_websocket.h"
#endif

#ifdef USE_CAPTDNS
#include "captdns.h"
#endif

#ifdef USE_OVERLAY
#include "overlay.h"
#endif

#define ifcmp(a)  if(rom_xstrcmp(cstr, a))

extern int rom_atoi(const char *);
#undef atoi
#define atoi rom_atoi

extern struct netif xnetif[NET_IF_NUM]; /* network interface structure */

#include "web_auth.c"
#include "web_srv.c"
#include "web_utils.c"
#include "web_websocket.c"
#include "websock.c"

//#include "web_int_vars.c"
//#include "web_int_callbacks.c"

#endif // USE_WEB
