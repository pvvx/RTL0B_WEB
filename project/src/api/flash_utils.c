/*
 * flash_utils.c
 *
 *  Created on: 16/02/2018
 *      Author: pvvx
 */

#include "flash_utils.h"

flash_t flashobj;
unsigned int flash_id;

/*-------------------------------------------------------------------------------------
 Копирует данные из области align(4) (flash, registers, ...) в область align(1) (ram)
--------------------------------------------------------------------------------------*/
void copy_align4_to_align1(unsigned char * pd, void * ps, unsigned int len)
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
/*------------------------------------------------------------------------------------
 Копирует данные из области align(1) (ram) в область align(4) (flash, registers)
--------------------------------------------------------------------------------------*/
void copy_align1_to_align4(void * pd, unsigned char * ps, unsigned int len)
{
	union {
		unsigned char uc[4];
		unsigned int ud;
	}tmp;
	unsigned int *p = (unsigned int *)(((unsigned int)pd) & (~3));
	unsigned int xlen = (unsigned int)pd & 3;
//	unsigned int size = len;
	if(xlen) {
		tmp.ud = *p;
		while (len)  {
			len--;
			tmp.uc[xlen++] = *ps++;
			if(xlen & 4) break;
		}
		*p++ = tmp.ud;
	}
	xlen = len >> 2;
	while(xlen) {
		tmp.uc[0] = *ps++;
		tmp.uc[1] = *ps++;
		tmp.uc[2] = *ps++;
		tmp.uc[3] = *ps++;
		*p++ = tmp.ud;
		xlen--;
	}
	if(len & 3) {
		tmp.ud = *p;
		tmp.uc[0] = ps[0];
		if(len & 2) {
			tmp.uc[1] = ps[1];
			if(len & 1) {
				tmp.uc[2] = ps[2];
			}
		}
		*p = tmp.ud;
	}
//	return size;
}
/*------------------------------------------------------------------------------------
 Запись байта в область align(4) (flash, registers)
--------------------------------------------------------------------------------------*/
void write_align4_chr(unsigned char *pd, unsigned char c)
{
	union {
		unsigned char uc[4];
		unsigned int ud;
	}tmp;
	unsigned int *p = (unsigned int *)((unsigned int)pd & (~3));
	unsigned int xlen = (unsigned int)pd & 3;
	tmp.ud = *p;
	tmp.uc[xlen] = c;
	*p = tmp.ud;
}
/*------------------------------------------------------------------------------------
 Чтение байта из области align(4) (flash, registers)
--------------------------------------------------------------------------------------*/
unsigned char get_align4_chr(const unsigned char *ps)
{
	return (*((unsigned int *)((unsigned int)ps & (~3))))>>(((unsigned int)ps & 3) << 3);
}
/*------------------------------------------------------------------------------------
 Сравнение данных в области align(4) (flash, registers, ...) с областью align(1) (ram)
--------------------------------------------------------------------------------------*/
int cmp_align1_align4(unsigned char * pd, void * ps, unsigned int len)
{
	union {
		unsigned char uc[4];
		unsigned int ud;
	}tmp;
	unsigned int *p = (unsigned int *)((unsigned int)ps & (~3));
	unsigned int xlen = (unsigned int)ps & 3;
	if(xlen) {
		tmp.ud = *p++;
		while (len)  {
			len--;
			if(*pd++ != tmp.uc[xlen++]) return 1;
			if(xlen & 4) break;
		}
	}
	xlen = len >> 2;
	while(xlen) {
		tmp.uc[0] = *pd++;
		tmp.uc[1] = *pd++;
		tmp.uc[2] = *pd++;
		tmp.uc[3] = *pd++;
		if(*p++ != tmp.ud) return 1;
		xlen--;
	}
	if(len & 3) {
		tmp.ud = *p;
		if(pd[0] != tmp.uc[0]) return 1;
		if(len & 2) {
			if(pd[1] != tmp.uc[1]) return 1;
			if(len & 1) {
				if(pd[2] != tmp.uc[2]) return 1;
			}
		}
	}
	return 0;
}

/*------------------------------------------------------------------------------------
 Получить id flash
--------------------------------------------------------------------------------------*/
unsigned int flash_get_id(void)
{
		if(!flash_id) {
			flash_read_id(&flashobj, (u8 *)&flash_id, 4);
		}
		return flash_id;
}

/*------------------------------------------------------------------------------------
 Получить размер Flash в байтах
--------------------------------------------------------------------------------------*/
unsigned int flash_get_size(void)
{
		if(!flash_id) {
			flash_read_id(&flashobj, (u8 *)&flash_id, 4);
		}
		return 1 << ((flash_id >> 16) & 0x0ff);
}
