/******************************************************************************
 * FileName: flash_eep.h
 * Description: FLASH
 * Alternate SDK 
 * Author: PV`
 * (c) PV` 2015
*******************************************************************************/
#ifndef __FLASH_UTILS_H_
#define __FLASH_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------
#include <platform_opts.h>
#include "flash_api.h"
#include "flash_eep.h"
#include "device_lock.h"
//-----------------------------------------------------------------------------
#define _flash_mutex_lock() device_mutex_lock(RT_DEV_LOCK_FLASH)
#define _flash_mutex_unlock() device_mutex_unlock(RT_DEV_LOCK_FLASH)
//-----------------------------------------------------------------------------
#define _flash_read(addr, len, buf) copy_align4_to_align1((unsigned char *)buf, (void *)(SPI_FLASH_BASE) + addr, (unsigned int)len)
#define _flash_read_dword(addr)  HAL_READ32(SPI_FLASH_BASE, addr) // (*((volatile u32*)(SPI_FLASH_BASE) + addr))
#define _flash_read_byte(addr) get_align4_chr((void *)(SPI_FLASH_BASE) + addr) 
#define _flash_erase_sector(n) flash_erase_sector(&flashobj, n)
#define _flash_write(addr, len, buf) flash_stream_write(&flashobj, (u32)addr, (u32)len, (u8 *)buf)
#define _flash_write_dword(addr, dw) flash_write_word(&flashobj, (u32)addr, (u32)dw)
#define _flash_memcmp(addr, len, buf) cmp_align1_align4((unsigned char *)buf, (void *)(SPI_FLASH_BASE) + addr, (unsigned int)len)
//-----------------------------------------------------------------------------
extern flash_t flashobj;
/* Копирует данные из области align(4) (flash, registers, ...) в область align(1) (ram) */
void copy_align4_to_align1(unsigned char * pd, void * ps, unsigned int len);
/* Копирует данные из области align(1) (ram) в область align(4) (flash, registers) */
void copy_align1_to_align4(void * pd, unsigned char * ps, unsigned int len);
/* Запись байта в область align(4) (flash, registers) */
void write_align4_chr(unsigned char *pd, unsigned char c);
/* Чтение байта из области align(4) (flash, registers) */
unsigned char get_align4_chr(const unsigned char *ps);
/* Сравнение данных в области align(4) (flash, registers, ...) с областью align(1) (ram) */
int cmp_align1_align4(unsigned char * pd, void * ps, unsigned int len);
//-----------------------------------------------------------------------------
/* Получить размер Flash в байтах */
unsigned int flash_get_size(void);
/* Получить Flash ID */
unsigned int flash_get_id(void);
extern unsigned int flash_id;
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif


#endif /* __FLASH_UTILS_H_ */
