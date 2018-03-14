/******************************************************************************
 * FileName: flash_eep.h
 * Description: FLASH
 * Alternate SDK 
 * Author: PV`
 * (c) PV` 2015
*******************************************************************************/
#ifndef __FLASH_EEP_H_
#define __FLASH_EEP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <basic_types.h>
//#include <FreeRTOS.h>
//#include <queue.h>

//-----------------------------------------------------------------------------
#define FMEMORY_SCFG_BANK_SIZE	FLASH_SECTOR_SIZE // размер сектора, 4096 bytes
#define FMEMORY_SCFG_BANKS 		4 // кол-во секторов для работы 2..
#define FMEMORY_SCFG_BASE_ADDR	(0x8000 - (FMEMORY_SCFG_BANKS*FMEMORY_SCFG_BANK_SIZE)) // = 0x6000
//-----------------------------------------------------------------------------
enum eFMEMORY_ERRORS {
	FMEM_NOT_FOUND = -1,	//  -1 - не найден
	FMEM_FLASH_ERR = -2,	//  -2 - flash rd/wr/erase error
	FMEM_ERROR = 	-3,		//  -3 - error
	FMEM_OVR_ERR = 	-4,		//  -4 - переполнение FMEMORY_SCFG_BANK_SIZE
	FMEM_MEM_ERR = 	-5		//  -5 - heap alloc error
};
//-----------------------------------------------------------------------------
// extern QueueHandle_t flash_mutex;
signed short flash_read_cfg(void *ptr, unsigned short id, unsigned short maxsize); // возврат: размер объекта последнего сохранения, -1 - не найден, -2 - error
bool flash_write_cfg(void *ptr, unsigned short id, unsigned short size);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif


#endif /* __FLASH_EEP_H_ */
