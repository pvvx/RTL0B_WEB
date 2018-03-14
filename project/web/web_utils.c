/*
 * web_utils.c
 *
 *  Created on: 25 дек. 2014 г.
 *      Author: PV`
 */
#ifndef COMPILE_SCI  // Use Single Compilation Unit "web"
#include "user_config.h"
#include "autoconf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "web/web_utils.h"
#endif // COMPILE_SCI

extern size_t rtl_strlen(const uint8_t *str);
extern void * rtl_memset(void * m , int c , size_t n);
extern int rtl_strncmp(const char *s1, const char *s2, size_t n);

/******************************************************************************
* xstrcpy() из сегментов flash и IRAM с возвратом размера строки:
* на выходе размер строки, без учета терминатора '\0'
*******************************************************************************/
int ICACHE_RAM_ATTR rom_xstrcpy(char * pd, const char * ps)
{
#if 0
	union {
		unsigned char uc[4];
		unsigned int ud;
	}tmp;
	if(ps == 0 || pd == 0) return (0);
	*pd = 0;
	unsigned int len = 0;
	unsigned int *p = (unsigned int *)((unsigned int)ps & (~3));
	unsigned int xlen = (unsigned int)ps & 3;
	while(1) {
		tmp.ud = *p++;
		do {
			if((*pd++ = tmp.uc[xlen++]) == 0) return len;
			len++;
			xlen &= 3;
		} while(xlen);
	}
#else
	int len = 0;
	while((*pd++ = *ps++) != 0) len++;
	return len;
#endif
}
/******************************************************************************
* сравнение строки в ram со строкой в сегменте flash и IRAM
*  = 1 если шаблон совпадает
*******************************************************************************/
int ICACHE_RAM_ATTR rom_xstrcmp(char * pd, const char * ps)
{
#if 0
	union {
		unsigned char uc[4];
		unsigned int ud;
	}tmp;
	if(ps == 0 || pd == 0) return 0;
	unsigned int *p = (unsigned int *)((unsigned int)ps & (~3));
	unsigned int xlen = (unsigned int)ps & 3;
	while(1) {
		tmp.ud = *p++;
		do {
			if(tmp.uc[xlen] == 0) return 1;
			if(tmp.uc[xlen++] != *pd || *pd++ == 0) return 0;
			xlen &= 3;
		} while(xlen);
	}
#else
	while(*ps) {
		if(*pd++ != *ps++) return 0;
	}
	return 1;
#endif
}
/******************************************************************************
 * rom_atoi
*******************************************************************************/
int ICACHE_FLASH_ATTR rom_atoi(const char *s)
{
	int n=0, neg=0;
	while (*s == ' ') s++;
	switch (*s) {
	case '-': neg=1;
	case '+': s++;
	}
	/* Compute n as a negative number to avoid overflow on INT_MIN */
	while (*s >= '0' && *s <= '9')
		n = 10*n - (*s++ - '0');
	return neg ? n : -n;
}

/******************************************************************************
 * get_seg_id()
*******************************************************************************/
const char * const txt_tab_seg[] = {
		"ROM"		// 0
		"FLASH",	// 1	// -> flash On
		"SRAM",		// 2
		"SOC",		// 3	// protected !
		"CPU",		// 4	// protected !
		"UNK",		// 5
		};

const uint32_t tab_seg_def[] = {
		0x00000000, 0x00080000,
		0x08000000, 0x10000000, // SPI_FLASH_BASE
		0x10000000, 0x10040000,
		0x40000000, 0x40800000, // SYSTEM_CTRL_BASE
		0xE0000000, 0xE0010000,
		0x00000000, 0xFFFFFFFF
};

SEG_ID get_seg_id(uint32_t addr, int32_t size) {
	SEG_ID ret = SEG_ID_ERR;
	uint32_t * ptr = (uint32_t *) &tab_seg_def;
	if (size > 0) {
		do {
			ret++;
			if (addr >= ptr[0] && addr + size <= ptr[1]) {
				return ret;
			};
			ptr += 2;
		} while (ret < SEG_ID_MAX);
	};
	return 0;
}
/******************************************************************************
 * copy_align4
 * копирует данные из области кеширования flash и т.д.
*******************************************************************************/
/*
void ICACHE_FLASH_ATTR copy_align4(void *ptrd, void *ptrs, uint32_t len)
{
	union {
		uint8_t uc[4];
		uint32_t ud;
	}tmp;
	uint8_t *pd = ptrd;
	uint32_t *p = (uint32_t *)((uint32_t)ptrs & (~3));
	uint32_t xlen = ((uint32_t)ptrs) & 3;
	if(xlen) {
		if(((uint32_t)p >= 0x08000000)&&((uint32_t)p < 0x10000000)) tmp.ud = *p++;
		else {
			tmp.ud = 0;
			p++;
		}
		while (len)  {
			*pd++ = tmp.uc[xlen++];
			len--;
			if(xlen >= 4) break;
		}
	}
	xlen = len >> 2;
	while(xlen) {
		if(((uint32_t)p >= 0x08000000)&&((uint32_t)p < 0x10000000)) tmp.ud = *p++;
		else {
			tmp.ud = 0;
			p++;
		}
		*pd++ = tmp.uc[0];
		*pd++ = tmp.uc[1];
		*pd++ = tmp.uc[2];
		*pd++ = tmp.uc[3];
		xlen--;
	}
	len &= 3;
	if(len) {
		if(((uint32_t)p >= 0x08000000)&&((uint32_t)p < 0x10000000)) tmp.ud = *p;
		else tmp.ud = 0;
		uint8_t * ptmp = tmp.uc;
		while (len--)  *pd++ = *ptmp++;
	}
}
*/
/******************************************************************************
 * FunctionName : hextoul
*******************************************************************************/
// bool conv_str_hex(uint32_t * dest, uint8_t *s);
uint32_t ICACHE_FLASH_ATTR hextoul(uint8_t *s)
{
/*
	uint32_t val;
	if(!conv_str_hex(&val, s)) return 0;
	return val;
*/
	uint32_t val = 0;
          while (*s)
          {
               if (*s >= '0' && *s <= '9')
               {
                 val <<= 4;
                 val |= *s - '0';
               }
               else if (*s >= 'A' && *s <= 'F')
               {
                 val <<= 4;
                 val |= *s - 'A' + 10;
               }
               else if (*s >= 'a' && *s <= 'f')
               {
                 val <<= 4;
                 val |= *s - 'a' + 10;
               }
               else break;
               s++;
          };
          return val;
}
/******************************************************************************
 * FunctionName : ahextoul
*******************************************************************************/
// bool convert_para_str(uint32_t * dest, uint8_t *s);
uint32_t ICACHE_FLASH_ATTR ahextoul(uint8_t *s)
{
/*
	uint32_t ret;
	if(!convert_para_str(&ret, s)) return 0;
	return ret;
*/
	if((s[0]=='0') && ((s[1] | 0x20) =='x')) return hextoul(s+2);
	return rom_atoi(s);
}
/******************************************************************************
 * FunctionName : cmpcpystr
 * Description  : выбирает слово из строки текста с заданными начальным символом
 *                и конечным терминатором. Терминатор и стартовый символ не копирует, если заданы.
 * Parameters   : При задании начального символа = '\0' берется любой символ (>' ').
                  Копирует до символа <' ' или терминатора.
                  Задается ограничение размера буфера для копируемого слова (с дописыванием в буфер '\0'!).
 * Returns      : Зависит от значения терминатора, указывает на терминатор в строке,
                  если терминатор найден.
                  Если NULL, то начальный или конечный терминатор не найден.
*******************************************************************************/
uint8_t * ICACHE_FLASH_ATTR cmpcpystr(uint8_t *pbuf, uint8_t *pstr, uint8_t a, uint8_t b, uint16_t len)
{
            if(len == 0) pbuf = NULL;
            if(pstr == NULL) {
              if(pbuf != NULL) *pbuf='\0';
              return NULL;
            };
            uint8_t c;
            do {
              c = *pstr;
              if(c < ' ') { // строка кончилась
                if(pbuf != NULL) *pbuf='\0';
                return NULL; // id не найден
              };
              if((a == '\0')&&(c > ' ')) break; // не задан -> любой символ
              pstr++;
              if(c == a) break; // нашли стартовый символ (некопируемый в буфер)
            }while(1);
            if(pbuf != NULL) {
              while(len--) {
                c = *pstr;
                if(c == b) { // нашли терминирующий символ (некопируемый в буфер)
                  *pbuf='\0';
                  return pstr; // конечный терминатор найден
                };
//                if(c <= ' ') { // строка кончилась или пробел
                if(c < ' ') { // строка кончилась или пробел
                  *pbuf='\0';
                  return NULL; // конечный терминатор не найден
                };
                pstr++;
                *pbuf++ = c;
              };
              *--pbuf='\0'; // закрыть буфер
            };
            do {
              c = *pstr;
              if(c == b) return pstr; // нашли терминирующий символ
//              if(c <= ' ') return NULL; // строка кончилась
              if(c < ' ') return NULL; // строка кончилась
              pstr++;
            }while(1);
}
/******************************************************************************
 * FunctionName : str_array
 * Набирает из строки s массив слов в buf в кол-ве до max_buf
 * возврат - кол-во переменных в строке
 * Разделитель переменных в строке ','
 * Если нет переменной, то пропускает изменение в buf
 * Примеры:
 * Строка "1,2,3,4" -> buf = 0x01 0x02 0x03 0x04
 * Строка "1,,3," -> buf = 0x01 (не изменено) 0x03 (не изменено)
*******************************************************************************/
uint32_t ICACHE_FLASH_ATTR str_array(uint8_t *s, uint32_t *buf, uint32_t max_buf)
{
	uint32_t ret = 0;
	uint8_t *sval = NULL;
	while(max_buf > ret) {
		if(sval == NULL) {
			if (*s == '-' && s[1] >= '0' && s[1] <= '9') {
				sval = s;
				s++;
			}
			else if (*s >= '0' && *s <= '9') sval = s;
		}
		if(*s == ',' || *s <= ')') {
			if(sval != NULL) {
				*buf = ahextoul(sval);
				sval = NULL;
			}
			buf++;
			ret++;
			if(*s < ')') return ret;
		}
		s++;
	}
	return ret;
}
uint32_t ICACHE_FLASH_ATTR str_array_w(uint8_t *s, uint16_t *buf, uint32_t max_buf)
{
	uint32_t ret = 0;
	uint8_t *sval = NULL;
	while(max_buf > ret) {
		if(sval == NULL) {
			if (*s == '-' && s[1] >= '0' && s[1] <= '9') {
				sval = s;
				s++;
			}
			else if (*s >= '0' && *s <= '9') sval = s;
		}
		if(*s == ',' || *s <= ')') {
			if(sval != NULL) {
				*buf = ahextoul(sval);
				sval = NULL;
			}
			buf++;
			ret++;
			if(*s < ')') return ret;
		}
		s++;
	}
	return ret;
}
uint32_t ICACHE_FLASH_ATTR str_array_b(uint8_t *s, uint8_t *buf, uint32_t max_buf)
{
	uint32_t ret = 0;
	uint8_t *sval = NULL;
	while(max_buf > ret) {
		if(sval == NULL) {
			if (*s == '-' && s[1] >= '0' && s[1] <= '9') {
				sval = s;
				s++;
			}
			else if (*s >= '0' && *s <= '9') sval = s;
		}
		if(*s == ',' || *s == '.' || *s <= ')') {
			if(sval != NULL) {
				*buf = ahextoul(sval);
				sval = NULL;
			}
			buf++;
			ret++;
			if(*s < ')') return ret;
		}
		s++;
	}
	return ret;
}
/******************************************************************************
 * FunctionName : strtmac
*******************************************************************************/
int ICACHE_FLASH_ATTR strtomac(uint8_t *s, uint8_t *macaddr)
{
	uint8_t * ptrm = macaddr;
	int slen = rtl_strlen(s);
	uint8_t * ptr = s;
	if(slen == (6*2 + 5) || slen == (6*2)) {
		uint8_t pbuf[4];
		pbuf[2] = 0;
		slen = 6;
		while(slen--) {
			pbuf[0] = ptr[0];
			pbuf[1] = ptr[1];
			*ptrm++ = (uint8_t)hextoul(pbuf);
			if(ptr[2] != ':') ptr +=2;
			else ptr +=3;
		}
		return 1;
	} else {
		rtl_memset(macaddr, 0xff, 6);
		return 0;
	}
}
/******************************************************************************
 * FunctionName : urldecode
*******************************************************************************/
int ICACHE_FLASH_ATTR urldecode(uint8_t *d, uint8_t *s, uint16_t lend, uint16_t lens)
{
	uint16_t ret = 0;
	if(s != NULL) while ((lens--) && (lend--) && (*s > ' ')) {
		if ((*s == '%')&&(lens > 1)) {
			s++;
			int i = 2;
			uint8_t val = 0;
			while(i--) {
				if (*s >= '0' && *s <= '9') {
					val <<= 4;
					val |= *s - '0';
				} else if (*s >= 'A' && *s <= 'F') {
					val <<= 4;
					val |= *s - 'A' + 10;
				} else if (*s >= 'a' && *s <= 'f') {
					val <<= 4;
					val |= *s - 'a' + 10;
				} else
					break;
				s++;
				lens--;
			};
			s--;
			*d++ = val;
		} else if (*s == '+')
			*d++ = ' ';
		else
			*d++ = *s;
		ret++;
		s++;
	}
	*d = '\0';
	return ret;
}
/******************************************************************************
 * FunctionName : urlencode
*******************************************************************************/
/*int ICACHE_FLASH_ATTR urlencode(uint8_t *d, uint8_t *s, uint16_t lend, uint16_t lens)
{
	uint16_t ret = 0;
	if(s != NULL) while ((lens--) && (lend--) && (*s != '\0')) {
		if ( (48 <= *s && *s <= 57) //0-9
				|| (65 <= *s && *s <= 90) //abc...xyz
				|| (97 <= *s && *s <= 122) //ABC...XYZ
				|| (*s == '~' || *s == '!' || *s == '*' || *s == '(' || *s == ')' || *s == '\'')) {
			*d++ = *s++;
			ret++;
		} else {
			if(lend >= 3) {
				ret += 3;
				lend -= 3;
				*d++ = '%';
				uint8_t val = *s >> 4;
				if(val <= 9) val += '0';
				else val += 0x41 - 10;
				*d++ = val;
				val = *s++ & 0x0F;
				if(val <= 9) val += '0';
				else val += 0x41 - 10;
				*d++ = val;
			}
			else break;
		}
	}
	*d = '\0';
	return ret;
}*/
/******************************************************************************
 * FunctionName : htmlcode
*******************************************************************************/
int ICACHE_FLASH_ATTR htmlcode(uint8_t *d, uint8_t *s, uint16_t lend, uint16_t lens)
{
	uint16_t ret = 0;
	if(s != NULL) while ((lens--) && (lend--) && (*s != '\0')) {
		if ( *s == 0x27 ) { // "'" &apos;
			if(lend >= 6) {
				ret += 6;
				lend -= 6;
				s++;
				*d++ = '&';
				*d++ = 'a';
				*d++ = 'p';
				*d++ = 'o';
				*d++ = 's';
				*d++ = ';';
			}
			else break;
		} else if ( *s == '"' ) { // &quot;
			if(lend >= 6) {
				ret += 6;
				lend -= 6;
				s++;
				*d++ = '&';
				*d++ = 'q';
				*d++ = 'u';
				*d++ = 'o';
				*d++ = 't';
				*d++ = ';';
			}
			else break;
		} else if ( *s == '&' ) { // &amp;
			if(lend >= 5) {
				ret += 5;
				lend -= 5;
				s++;
				*d++ = '&';
				*d++ = 'a';
				*d++ = 'm';
				*d++ = 'p';
				*d++ = ';';
			}
			else break;
		} else if ( *s == '<' ) { // &lt;
			if(lend >= 4) {
				ret += 4;
				lend -= 4;
				s++;
				*d++ = '&';
				*d++ = 'l';
				*d++ = 't';
				*d++ = ';';
			}
			else break;
		} else if ( *s == '>' ) { // &gt;
			if(lend >= 4) {
				ret += 4;
				lend -= 4;
				s++;
				*d++ = '&';
				*d++ = 'g';
				*d++ = 't';
				*d++ = ';';
			}
			else break;
		} else {
			*d++ = *s++;
			ret++;
		}
	}
	*d = '\0';
	return ret;
}
//=============================================================================

uint8_t* ICACHE_FLASH_ATTR
web_strnstr(const uint8_t* buffer, const uint8_t* token, int len)
{
  const uint8_t* p;
  int tokenlen = rtl_strlen(token);
  if (tokenlen == 0) {
    return (uint8_t *)buffer;
  };
  for (p = buffer; *p && (p + tokenlen <= buffer + len); p++) {
    if ((*p == *token) && (rtl_strncmp(p, token, tokenlen) == 0)) {
      return (uint8_t *)p;
    };
  };
  return NULL;
}
//=============================================================================
static const uint8_t base64map[128] ICACHE_RODATA_ATTR =
{
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,
		 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,  0,255,255,
		255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
		 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,
		255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255
};
//=============================================================================
bool ICACHE_FLASH_ATTR base64decode(const uint8_t *in, int len, uint8_t *out, int *outlen)
{
	uint8_t *map = (uint8_t *) base64map;
    int g, t, x, y, z;
    uint8_t c;
    g = 3;
    for (x = y = z = t = 0; x < len; x++) {
        if ((c = map[in[x]&0x7F]) == 0xff) continue;
        if (c == 254) {  /* this is the end... */
            c = 0;
            if (--g < 0) return false;
        }
        else if (g != 3) return false; /* only allow = at end */
        t = (t<<6) | c;
        if (++y == 4) {
            out[z++] = (uint8_t)((t>>16)&255);
            if (g > 1) out[z++] = (uint8_t)((t>>8)&255);
            if (g > 2) out[z++] = (uint8_t)(t&255);
            y = t = 0;
        }
        /* check that we don't go past the output buffer */
        if (z > *outlen) return false;
    }
    if (y != 0) return false;
    *outlen = z;
    return true;
}
//=============================================================================
/* Table 6-bit-index-to-ASCII used for base64-encoding */
const uint8_t base64_table[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
  'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
  'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  '+', '/'
};
// ld: PROVIDE ( base64_table = 0x3FFFD600 );
//extern const uint8_t base64_table[];
//=============================================================================
/** Base64 encoding */
size_t ICACHE_FLASH_ATTR base64encode(char* target, size_t target_len, const char* source, size_t source_len)
{
  size_t i;
  int8_t j;
  size_t target_idx = 0;
  size_t longer = 3 - (source_len % 3);
  size_t source_len_b64 = source_len + longer;
  size_t len = (((source_len_b64) * 4) / 3);
  uint8_t x = 5;
  uint8_t current = 0;

  if(target == NULL || target_len < len) return 0;

  for (i = 0; i < source_len_b64; i++) {
    uint8_t b = (i < source_len ? source[i] : 0);
    for (j = 7; j >= 0; j--, x--) {
      uint8_t shift = ((b & (1 << j)) != 0) ? 1 : 0;
      current |= shift << x;
      if (x == 0) {
        target[target_idx++] = base64_table[current];
        x = 6;
        current = 0;
      }
    }
  }
  for (i = len - longer; i < len; i++) {
    target[i] = '=';
  }
  return len;
}
/*
//=============================================================================
void ICACHE_FLASH_ATTR print_hex_dump(uint8_t *buf, uint32_t len, uint8_t k)
{
	if(!system_get_os_print()) return; // if(*((uint8_t *)(0x3FFE8000)) == 0) return;
	uint32_t ss[2];
	ss[0] = 0x78323025; // "%02x"
	ss[1] = k;	// ","...'\0'
	uint8_t* ptr = buf;
	while(len--) {
		if(len == 0) ss[1] = 0;
		ets_printf((uint8_t *)&ss[0], *ptr++);
	}
}
*/
//=============================================================================
#define LowerCase(a) ((('A' <= a) && (a <= 'Z')) ? a + 32 : a)

char* ICACHE_FLASH_ATTR word_to_lower_case(char* text) {
	for(; *text ==' '; text++);
	char* p = text;
	for (; *p >= ' '; p++) {
		*p = LowerCase(*p);
	}
	return text;
}
#if 0
//=============================================================================
/* char UpperCase(char ch) {
	return (('a' <= ch) && (ch <= 'z')) ? ch - 32 : ch; }*/
#define UpperCase(a) ((('a' <= a) && (a <= 'z')) ? a - 32 : a)

char* ICACHE_FLASH_ATTR str_to_upper_case(char* text) {
	char* p = text;
	for (; *p; ++p) {
		*p = UpperCase(*p);
	}
	return text;
}
#endif

//=============================================================================
void *zalloc(size_t xWantedSize) {
	void * prt = pvPortMalloc(xWantedSize);
	if (prt)
		rtl_memset(prt, 0, xWantedSize);
	return prt;
}

