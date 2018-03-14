#include <platform_opts.h>

#if 1 //def CONFIG_AT_USR

//#include "ameba_soc.h"
#include "device.h"
#include "FreeRTOS.h"
#include "task.h"
//#include "semphr.h"
//#include "freertos_pmu.h"
//#include "rtl_lib.h"

#include "flash_utils.h"
#include "sleep_ex_api.h"

#include "osdep_service.h"
#include "console_api.h"


extern char str_rom_41ch3Dch0A[]; // "========================================\n" 41

//#define error_printf rtl_printf
#define LOCAL static
#define printf rtl_printf // DiagPrintf

//------------------------------------------------------------------------------
// Mem, Tasks info
//------------------------------------------------------------------------------
void fATST(int argc, unsigned char *argv[]) {
		(void) argc; (void) argv;
		printf("\nCLK CPU\t\t%d Hz\nRAM heap\t%d bytes\n",
				CPU_ClkGet(0), xPortGetFreeHeapSize());
		printf("\n");
#if (configGENERATE_RUN_TIME_STATS == 1)
		char *cBuffer = pvPortMalloc(512);
		if(cBuffer != NULL) {
			vTaskGetRunTimeStats((char *)cBuffer);
			printf("%s", cBuffer);
		}
		vPortFree(cBuffer);
#endif
#if defined(configUSE_TRACE_FACILITY) && (configUSE_TRACE_FACILITY == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS == 1)
	{
		char * pcWriteBuffer = malloc(1024);
		if(pcWriteBuffer) {
			vTaskList((char*)pcWriteBuffer);
			printf("\nTask List:\n");
			printf(str_rom_41ch3Dch0A); // "==========================================\n"
	        printf("Name\t  Status Priority HighWaterMark TaskNumber\n%s\n", pcWriteBuffer);
			free(pcWriteBuffer);
		}
	}
#endif
}
/*-------------------------------------------------------------------------------------
 Копирует данные из области align(4) (flash, registers, ...) в область align(1) (ram)
--------------------------------------------------------------------------------------*/
#if 1
extern void copy_align4_to_align1(unsigned char * pd, void * ps, unsigned int len);
#else
static void copy_align4_to_align1(unsigned char * pd, void * ps, unsigned int len)
{
	union {
		unsigned char uc[4];
		unsigned int ud;
	}tmp;
	unsigned int *p = (unsigned int *)((unsigned int)ps & (~3));
	unsigned int xlen = (unsigned int)ps & 3;
	//	unsigned int size = len;

	if(xlen) {
		tmp.ud = *p++;
		while (len)  {
			len--;
			*pd++ = tmp.uc[xlen++];
			if(xlen & 4) break;
		}
	}
	xlen = len >> 2;
	while(xlen) {
		tmp.ud = *p++;
		*pd++ = tmp.uc[0];
		*pd++ = tmp.uc[1];
		*pd++ = tmp.uc[2];
		*pd++ = tmp.uc[3];
		xlen--;
	}
	if(len & 3) {
		tmp.ud = *p;
		pd[0] = tmp.uc[0];
		if(len & 2) {
			pd[1] = tmp.uc[1];
			if(len & 1) {
				pd[2] = tmp.uc[2];
			}
		}
	}
	//	return size;
}
#endif

int print_hex_dump(uint8_t *buf, int len, unsigned char k) {
	uint32_t ss[2];
	ss[0] = 0x78323025; // "%02x"
	ss[1] = k;	// ","...'\0'
	uint8_t * ptr = buf;
	int result = 0;
	while (len--) {
		if (len == 0)
			ss[1] = 0;
		result += printf((uint8_t *) &ss, *ptr++);
	}
	return result;
}

extern char str_rom_hex_addr[]; // in *.ld "[Addr]   .0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .A .B .C .D .E .F\n" // 0x59316
void dump_bytes(uint32_t addr, int size)
{
	uint8_t buf[17];
	int symbs_line = sizeof(buf)-1;
	printf(str_rom_hex_addr);
	while (size) {
		if (symbs_line > size) symbs_line = size;
		printf("%08X ", addr);
		copy_align4_to_align1(buf, (void *) addr, symbs_line);
		print_hex_dump(buf, symbs_line, ' ');
		int i;
		for(i = 0 ; i < symbs_line ; i++) {
			if(buf[i] < 0x20 || buf[i] > 0x7E) {
				buf[i] = '.';
			}
		}
		buf[symbs_line] = 0;
		i = (sizeof(buf)-1) - symbs_line;
		while(i--) printf("   ");
		printf(" %s\r\n", buf);
		addr += symbs_line;
		size -= symbs_line;
	}
}
//------------------------------------------------------------------------------
// Dump byte register
//------------------------------------------------------------------------------
extern u32 Strtoul(
    IN  const u8 *nptr,
    IN  u8 **endptr,
    IN  u32 base
);

LOCAL void fATSB_(int argc, unsigned char *argv[])
{
	int size = 16;
	uint32_t addr = Strtoul(argv[1],0,16);
	if (argc > 2) {
		size = Strtoul(argv[2],0,10);
		if (size <= 0 || size > 16384)
			size = 16;
	}
	if(addr + size > SPI_FLASH_BASE) {
//		flash_turnon();
		dump_bytes(addr, size);
//		SpicDisableRtl8195A();
	}
	else {
		dump_bytes(addr, size);
	}
}
//------------------------------------------------------------------------------
// Deep sleep
//------------------------------------------------------------------------------
LOCAL void fATDS(int argc, unsigned char *argv[])
{
    uint32_t sleep_ms = 10000;
    if(argc > 1) sleep_ms = atoi(argv[1]);
    deepsleep_ex(DSLEEP_WAKEUP_BY_TIMER, sleep_ms);
}
/*------------------------------------------------------------------------------
 * power saving mode
 *----------------------------------------------------------------------------*/
LOCAL void fATSP_(int argc, unsigned char *argv[])
{
	if(argc > 2) {
		switch (argv[1][0]) {
		case 'a': // acquire
		{
			pmu_acquire_wakelock(1<<atoi(argv[2]));
			break;
		}
		case 'r': // release
		{
			pmu_release_wakelock(1<<atoi(argv[2]));
			break;
		}
		};
	};
	printf("WakeLock Status %d\n", pmu_get_wakelock_status());
}
/*------------------------------------------------------------------------------
 * Flash info
 *----------------------------------------------------------------------------*/
LOCAL void fATFI(int argc, unsigned char *argv[]) {
	int size = flash_get_size();
	printf("Flash ID: 0x%06x, Size: %d bytes, Status: 0x%x\n", flash_id, size, flash_get_status(&flashobj));
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
MON_RAM_TAB_SECTION MON_COMMAND_TABLE console_commands_at[] = {
		{"DB", 1, fATSB_, "=<ADDRES(hex)>[,COUNT(dec)]: Dump byte register"},
		{"FID", 0, fATFI, ": Flash info"},
		{"MEM", 0, fATST, ": Memory info"},
		{"DPSLP", 0, fATDS, "=[TIME(ms)]: Deep sleep"},
		{"WAKE", 0, fATSP_, "=<a,r>,<wakelock_status:0..31>: Power"}
};

#endif //#ifdef CONFIG_AT_USR
