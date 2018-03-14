/*
 * console_api.c
 *
 *  Created on: 24/02/17
 *      Author: pvvx
 */
//======================================================
#ifndef LOGUART_STACK_SIZE
#define	LOGUART_STACK_SIZE	768 // USE_MIN_STACK_SIZE modify from 1024 to 128
#endif
#define	CONSOLE_PRIORITY 0
//======================================================
#include "ameba_soc.h"
#include "osdep_service.h"
#include <stdarg.h>
#include "rtl_consol.h"
#include "strproc.h"
#include "console_api.h"

#if defined(CONFIG_KERNEL) && !TASK_SCHEDULER_DISABLED
#else
#error "Define CONFIG_KERNEL & TASK_SCHEDULER_DISABLED = 0!"
#endif
#ifndef CONFIG_UART_LOG_HISTORY
#error "Define CONFIG_UART_LOG_HISTORY!"
#endif
//======================================================
extern volatile UART_LOG_CTL	UartLogCtl;
extern volatile UART_LOG_CTL	*pUartLogCtl;
extern uint8_t	*ArgvArray[MAX_ARGV];
extern UART_LOG_BUF	UartLogBuf;
#ifdef CONFIG_UART_LOG_HISTORY
extern uint8_t	UartLogHistoryBuf[UART_LOG_HISTORY_LEN][UART_LOG_CMD_BUFLEN];
#endif
#define ADD_ROM_MON 0
//======================================================
// #define USE_ROM_CONSOLE
//======================================================
_LONG_CALL_ extern uint8_t UartLogCmdChk( IN uint8_t RevData,
		IN UART_LOG_CTL *prvUartLogCtl, IN uint8_t EchoFlag);

_LONG_CALL_ extern void ArrayInitialize( IN uint8_t *pArrayToInit,
		IN uint8_t ArrayLen, IN uint8_t InitValue);

_LONG_CALL_ extern void UartLogHistoryCmd( IN uint8_t RevData,
		IN UART_LOG_CTL *prvUartLogCtl, IN uint8_t EchoFlag);

//_LONG_CALL_ extern void UartLogCmdExecute(IN PUART_LOG_CTL pUartLogCtlExe);
//======================================================
#ifdef CONFIG_KERNEL
//volatile uint8_t Consol_TaskRdy;
#define Consol_TaskRdy p->Resvd
_sema Consol_Sema;
#else
// Since ROM code will reference this typedef, so keep the typedef same size
uint8_t Consol_TaskRdy;
void *Consol_Sema;
#endif
//======================================================
extern PMON_COMMAND_TABLE UartLogMonCmdTable[];
extern int UartLogMonCmdTableSize;
//======================================================
//<Function>:  UartLogIrqHandleRam
//<Usage   >:  To deal with Uart-Log RX IRQ
//<Argus   >:  void
//<Return  >:  void
//<Notes   >:  NA
//======================================================
// overload original UartLogIrqHandle
//MON_RAM_TEXT_SECTION
void UartLogIrqHandleRam(void * Data) {
	uint32_t IrqEn = LOGUART_GetIMR(); // HAL_UART_READ32(UART_INTERRUPT_EN_REG_OFF)
	LOGUART_SetIMR(0); // HAL_UART_WRITE32(UART_INTERRUPT_EN_REG_OFF, 0)
	uint8_t UartReceiveData = LOGUART_GetChar(0); // if(flg) while(!(HAL_UART_READ32(UART_LINE_STATUS_REG_OFF)&1)); return HAL_UART_READ32(UART_REV_BUF_OFF);
	if (UartReceiveData == 0) {
		goto exit;
	}
	PUART_LOG_CTL p = (PUART_LOG_CTL) pUartLogCtl;
	//KB_ESC chk is for cmd history, it's a special case here.
	if (UartReceiveData == KB_ASCII_ESC) {
		// Esc detection is only valid in the first stage of boot sequence (few seconds)
		if (p->ExecuteEsc != 1) {
			p->ExecuteEsc = 1;
			p->EscSTS = 0;
		} else {
			//4 the input commands are valid only when the task is ready to execute commands
			if (p->BootRdy == 1
#ifdef CONFIG_KERNEL
					|| Consol_TaskRdy != 0
#endif
							) {
				if (p->EscSTS == 0) {
					p->EscSTS = 1;
				}
			} else {
				p->EscSTS = 0;
			}
		}
	} else if (p->EscSTS == 1) {
		if (UartReceiveData != KB_ASCII_LBRKT) { // '['
			p->EscSTS = 0;
		} else {
			p->EscSTS = 2;
		}
	} else {
		if (p->EscSTS == 2) {
			p->EscSTS = 0;
			if (UartReceiveData == 'A' || UartReceiveData == 'B') {
				UartLogHistoryCmd(UartReceiveData, (UART_LOG_CTL *) pUartLogCtl, 1);
			}
		} else {
			if (UartLogCmdChk(UartReceiveData, (UART_LOG_CTL *) pUartLogCtl, 1) == 2) {
				// check UartLog buffer to prevent from incorrect access
				if (p->pTmpLogBuf != NULL) {
					p->ExecuteCmd = 1;
#if defined(CONFIG_KERNEL) && !TASK_SCHEDULER_DISABLED
					if (Consol_TaskRdy)
						rtw_up_sema_from_isr((_sema *) &Consol_Sema);
#endif
				} else {
					ArrayInitialize(
							(uint8_t *) pUartLogCtl->pTmpLogBuf->UARTLogBuf,
							UART_LOG_CMD_BUFLEN, '\0');
				}
			}
		}
	}
exit:
	LOGUART_SetIMR(IrqEn); // HAL_UART_WRITE32(UART_INTERRUPT_EN_REG_OFF, IrqEn)
}
//======================================================
//<Function>:  GetArgvRam
//<Usage   >:  парсигн аргументов строки
//<Argus   >:  pstr - указатель на строку
//<Return  >:  кол-во аргументов
//<Notes   >:  2 формата:
// 1) cmd=arg1,arg2,...
// 2) cmd arg1 arg2
// arg может быть обрамлен '"' или '\''
// для передачи ' ' или ','.
// Начальные пробелы cmd или arg удаляются.
//======================================================
int GetArgvRam(IN u8 *pstr, u8** argv) {
	int arvc = 0;
//	u8** argv = ArgvArray;
	u8* p = pstr;
	u8 t = 0, n = ' ';
	int m = 0;
	while (*p != 0 && *p != '\r' && *p != '\n' && arvc < MAX_ARGV
			&& p < &pstr[UART_LOG_CMD_BUFLEN - 1]) {
		switch (m) {
		case 0: // wait cmd
			if (*p == ' ') {
//				*p = 0;
				break;
			}
			*argv++ = p;
			arvc++;
			m++;
			break;
		case 1: // test end cmd, type format parm
			if (*p == ' ') { // format cmd arg1 arg2 ...
				m++;
				*p = 0;
			} else if (*p == '=') { // "at" format cmd=arg1,arg2,...
				n = ',';
				m++;
				*p = 0;
			}
			break;
		case 2:	// wait start arg
			if (*p == ' ') {
				*p = 0;
				break;
			}
			if (*p == '"' || *p == '\'') {
				t = *p;
				m = 4;
				*p = 0;
				break;
			}
			*argv++ = p;
			arvc++;
			m++;
		case 3:	// end arg
			if (*p == n) { // ' ' or ','
				m = 2;
				*p = 0;
			}
			break;
		case 4:
			*argv++ = p;
			arvc++;
			m++;
		case 5:
			if (*p == t) { // '\'' or '"'
				m = 3;
				*p = 0;
			}
			break;
		}
		p++;
	}
	return arvc;
}
//======================================================
//<Function>:  RtlConsolTaskRam
//<Usage   >:  overload original RtlConsolTaskRam
//<Argus   >:  Data - указатель PUART_LOG_CTL
//<Return  >:  none
//<Notes   >:
//======================================================
//MON_RAM_TEXT_SECTION
void RtlConsolTaskRam(void *Data) {
	PUART_LOG_CTL p = (PUART_LOG_CTL) pUartLogCtl;
#ifdef USE_ROM_CONSOLE // show Help
	p->pTmpLogBuf->UARTLogBuf[0] = '?';
	p->pTmpLogBuf->BufCount = 1;
	p->ExecuteCmd = 1;
#endif
#ifdef CONFIG_KERNEL
	Consol_TaskRdy = 1;
#else
	pUartLogCtl->BootRdy = 1;
#endif
	do {
		//4 Set this for UartLog check cmd history
#if defined(CONFIG_KERNEL) && !TASK_SCHEDULER_DISABLED
		rtw_down_sema((_sema *)&Consol_Sema);
#endif
		if (p->ExecuteCmd) {
			//	UartLogCmdExecute(pUartLogCtl);
			int argc = GetArgvRam(p->pTmpLogBuf->UARTLogBuf, ArgvArray);
			if (argc) {
				StrUpr(ArgvArray[0]);
				PMON_COMMAND_TABLE pcmd = (PMON_COMMAND_TABLE)p->pCmdTbl;
#if  ADD_ROM_MON
				int flg = 1;
#endif
#ifdef USE_ROM_CONSOLE
				for(int i = 0; i < p->CmdTblSz; i++) {
#else
				while (pcmd->cmd) {
#endif
					if (prvStrCmp(ArgvArray[0], pcmd->cmd) == 0) {
#if  ADD_ROM_MON
						flg = 0;
#endif
						if (pcmd->ArgvCnt < argc) {
#ifdef USE_ROM_CONSOLE
							pcmd->func(argc-1, (uint8_t **) &ArgvArray[1]);
#else
							pcmd->func(argc, (uint8_t **) ArgvArray);
#endif
						} else {
#ifdef USE_ROM_CONSOLE
							DiagPrintf(pcmd->msg);
#else
							DiagPrintf("%s%s\n", pcmd->cmd, pcmd->msg);
#endif
						}
					}
					pcmd++;
				}
#if  ADD_ROM_MON
				if (flg) {
					pcmd = (PMON_COMMAND_TABLE) &UartLogRomCmdTable[1];
					int i = 6;
					while (i--) {
						if (prvStrCmp(ArgvArray[0], pcmd->cmd) == 0) {
							flg = 0;
							if (pcmd->ArgvCnt < argc) {
								pcmd->func(argc-1, (uint8_t **) &ArgvArray[1]);
							} else {
								DiagPrintf(pcmd->msg);
							}
						}
						pcmd++;
					}
					if (flg)
						DiagPrintf("cmd: %s - nothing!\n", ArgvArray[0]);
				}
#endif
//				pmu_release_wakelock(PMU_LOGUART_DEVICE);
			}
//			else
//				pmu_acquire_wakelock(PMU_LOGUART_DEVICE);
			pmu_set_sysactive_time(PMU_CONSOL_DEVICE, 10000);
			p->pTmpLogBuf->BufCount = 0;
			p->pTmpLogBuf->UARTLogBuf[0] = 0;
			LOGUART_PutChar('\r');
			LOGUART_PutChar('>');
			p->ExecuteCmd = 0;
		}
	} while (1);
}

/**
 * @brief    Set UartLog Baud Rate use baudrate val.
 * @param  BaudRate: Baud Rate Val, like 115200 (unit is HZ).
 * @retval  True/False
 */
int LOGUART_SetBaud(uint32_t BaudRate) {

	UART_INTConfig(UART2_DEV, RUART_IER_ERBI | RUART_IER_ELSI, DISABLE);
	UART_RxCmd(UART2_DEV, DISABLE);

	UART_SetBaud(UART2_DEV, BaudRate);

	UART_INTConfig(UART2_DEV, RUART_IER_ERBI | RUART_IER_ELSI, ENABLE);
	UART_RxCmd(UART2_DEV, ENABLE);

	return 1;
}

void LOGUART_SetBaud_FromFlash(void) {
	SYSTEM_DATA *SysData = (SYSTEM_DATA *) (SPI_FLASH_BASE + FLASH_SYSTEM_DATA_ADDR);

	/* 0xFFFFFFFF is 115200 */
	if (SysData->UlogRate != 0xFFFFFFFF) {
		DBG_8195A("LOGUART_SetBaud %d \n\n", SysData->UlogRate);
		LOGUART_SetBaud(SysData->UlogRate);
	}
}

void console_help(int argc, unsigned char *argv[]);
//======================================================
//<Function>:  console_init
//<Usage   >:  Initialize rtl console
//<Argus   >:  none
//<Return  >:  none
//<Notes   >:  delete rtl_concole.h from project
//======================================================
//MON_RAM_TEXT_SECTION
void console_init(void) {
	// Register Isr handle
	VECTOR_IrqUnRegister(UART_LOG_IRQ);
#ifdef USE_ROM_CONSOLE // use ROM Consol init & printf "<RTL8195A>"
	RtlConsolInit(RAM_STAGE, (uint32_t) 6, (void*) UartLogRomCmdTable);
#else
	UartLogBuf.BufCount = 0;
	ArrayInitialize(&UartLogBuf.UARTLogBuf[0], UART_LOG_CMD_BUFLEN, '\0');
	pUartLogCtl = &UartLogCtl;
	PUART_LOG_CTL p = (PUART_LOG_CTL) pUartLogCtl;
	p->NewIdx = 0;
	p->SeeIdx = 0;
	p->EscSTS = 0;
	p->BootRdy = 0;
	p->pTmpLogBuf = &UartLogBuf;
	p->CRSTS = 0;
	p->pHistoryBuf = UartLogHistoryBuf;
	p->pfINPUT = (void*) &DiagPrintf;
	p->pCmdTbl = (PCOMMAND_TABLE) UartLogMonCmdTable;
	p->CmdTblSz = UartLogMonCmdTableSize / 16;
	//6; // GetRomCmdNum()
#ifdef CONFIG_KERNEL
	Consol_TaskRdy = 0;
#endif
#endif
	p->RevdNo = UART_LOG_HISTORY_LEN;
#if defined(CONFIG_KERNEL)
	// Create a Semaphone
	rtw_init_sema((_sema*) &(Consol_Sema), 0);
//	Consol_TaskRdy = 0;
#endif
	// executing boot sequence
	p->ExecuteCmd = 0;
	p->ExecuteEsc = 1;		//don't check Esc anymore

	///	InterruptRegister(&UartIrqHandle);
	DIAG_UartReInit((IRQ_FUN) UartLogIrqHandleRam);
	NVIC_SetPriority(UART_LOG_IRQ, 10); /* this is rom_code_patch */
	LOGUART_SetBaud_FromFlash();
	if (pdTRUE
			!= xTaskCreate(RtlConsolTaskRam,
					(const signed char * const )"loguart", LOGUART_STACK_SIZE,
					NULL,
					tskIDLE_PRIORITY + CONSOLE_PRIORITY + PRIORITIE_OFFSET,
					NULL)) {
		DiagPrintf("Create Log UART Task Err!\n");
	}
}
#ifndef USE_ROM_CONSOLE
//======================================================
//<Function>:  console_help
//<Usage   >:  Initialize rtl console
//<Argus   >:  argc - кол-во аргуметов, argv - список аргументов
//<Return  >:  none
//<Notes   >:
//======================================================
extern char str_rom_41ch3Dch0A[]; // "=========================================================\n" 41 шт
_WEAK void console_help(int argc, unsigned char *argv[]) { 	// Help
	(void)argc;	(void)argv;

	DiagPrintf("CONSOLE COMMAND SET:\n");
	DiagPrintf(&str_rom_41ch3Dch0A[41 - 30]); //	DiagPrintf("==============================\n");
	PMON_COMMAND_TABLE pcmdtab = (PMON_COMMAND_TABLE) UartLogMonCmdTable;
	while (pcmdtab->cmd) {
#ifdef USE_ROM_CONSOLE
		DiagPrintf(pcmdtab->msg);
#else
		DiagPrintf("%s%s\n", pcmdtab->cmd, pcmdtab->msg);
#endif
		pcmdtab++;
	}
	DiagPrintf(&str_rom_41ch3Dch0A[41 - 30]); //	DiagPrintf("==============================\n");
#if ADD_ROM_MON
	pcmdtab = (PMON_COMMAND_TABLE) UartLogRomCmdTable;
	int i = 7;
	while (i--) {
#ifdef USE_ROM_CONSOLE
		DiagPrintf(pcmdtab->msg);
#else
		DiagPrintf("%s%s\n", pcmdtab->cmd, pcmdtab->msg);
#endif
		pcmdtab++;
	}
#endif
}

 void print_on(int argc, unsigned char *argv[])
 {
	(void)argc;
	ConfigDebugClose = argv[1][0]!='1';
 }

// REBOOT
void _CmdReboot(int argc, unsigned char *argv[]) {
//	CmdReboot(argc-1, argv[1]);
	if(argc && argv[1][0]=='1') BKUP_Set(0, BIT_UARTBURN_BOOT);
	BKUP_Set(0, BIT_SYS_RESET_HAPPEN);
	DiagPrintf("\r\nRebooting ...\r\n");
	NVIC_SystemReset();
}

//#define MON_RAM_TAB_SECTION	SECTION(".mon.tab.rodata")
// (!) размещается в специальном сегменте '.mon.tab*' (см. *.ld файл)
MON_RAM_TAB_SECTION MON_COMMAND_TABLE console_commands[] = {
		{"PR", 1, print_on, "=<1/0>: Printf on/off"},	// Help
		{"REBOOT", 0, _CmdReboot, ": <1 - uartburn>" },
		{ "?", 0, console_help, ": This Help" }	// Help
};
#endif
