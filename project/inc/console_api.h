/*
 * console.h
 *
 *  Created on: 08/01/2018.
 *      Author: pvvx
 */

#ifndef PROJECT_INC_CONSOLE_API_H_
#define PROJECT_INC_CONSOLE_API_H_
#include "basic_types.h"

typedef void (*cnfn_t)(int argc, unsigned char *argv[]);

typedef struct _MON_COMMAND_TABLE_ {
    const unsigned char* cmd;
    const int     ArgvCnt;
    cnfn_t func;
    const unsigned char* msg;
}MON_COMMAND_TABLE, *PMON_COMMAND_TABLE;


#define MON_RAM_TAB_SECTION	SECTION(".mon.tab.rodata")

void console_init(void);

#endif /* PROJECT_INC_CONSOLE_API_H_ */
