#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "main.h"
#include "wdt_api.h"
#include <example_entry.h>

extern void user_init_thrd(void * par);
//extern void console_init(void);
//extern int rtl_cryptoEngine_init(void);
extern int rtl_printf(const char *fmt, ...);
//extern void ReRegisterPlatformLogUart(void);

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
//	DBG_ERR_MSG_ON(_DBG_MISC_); // print MISC On
/*
	ConfigDebugErr = -1;
	ConfigDebugWarn = -1;
	ConfigDebugInfo = -1;
*/	
#if CONFIG_WDG_ON
	watchdog_init(CONFIG_WDG_ON);
	watchdog_start();
#endif

	/* wlan & user_start intialization */
	xTaskCreate(user_init_thrd, "user_init", 1024, NULL, tskIDLE_PRIORITY + 1 + PRIORITIE_OFFSET, NULL);

   	/*Enable Schedule, Start Kernel*/
#if defined(CONFIG_KERNEL) && !TASK_SCHEDULER_DISABLED
	#ifdef PLATFORM_FREERTOS
	vTaskStartScheduler();
	#endif
#else
	RtlConsolTaskRom(NULL);
#endif
	while(1);
	return 0;
}
