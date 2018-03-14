 /******************************************************************************
 * FileName: web_utils.h
 * Alternate SDK 
 * Author: PV`
 * (c) PV` 2015
*******************************************************************************/

#ifndef _INCLUDE_WEB_UTILS_H_
#define _INCLUDE_WEB_UTILS_H_

#define ICACHE_RAM_ATTR
#define ICACHE_FLASH_ATTR
#define ICACHE_RODATA_ATTR
#ifndef LOCAL
#define LOCAL static
#endif
#ifndef true
#define true (1)
#endif
#ifndef false
#define false (0)
#endif
#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif
#undef mMIN
#define mMIN(a, b)  ((a < b)? a : b)
#undef mMAX
#define mMAX(a, b)  ((a > b)? a : b)

typedef enum {
	SEG_ID_ROM = 0,
	SEG_ID_FLASH,
	SEG_ID_SRAM,
	SEG_ID_SOC,
	SEG_ID_CPU,
	SEG_ID_ERR,
	SEG_ID_MAX
} SEG_ID;

extern const uint32_t tab_seg_def[];
SEG_ID get_seg_id(uint32_t addr, int32_t size);

int rom_atoi(const char *s);
//void copy_align4(void *ptrd, void *ptrs, uint32_t len);
uint32_t hextoul(uint8_t *s);
uint32_t ahextoul(uint8_t *s);
uint8_t * cmpcpystr(uint8_t *pbuf, uint8_t *pstr, uint8_t a, uint8_t b, uint16_t len);
uint8_t * web_strnstr(const uint8_t* buffer, const uint8_t* token, int n);
bool base64decode(const uint8_t *in, int len, uint8_t *out, int *outlen);
size_t base64encode(char* target, size_t target_len, const char* source, size_t source_len);
int strtomac(uint8_t *s, uint8_t *macaddr);
//uint32_t strtoip(uint8_t *s); // ipaddr_addr();
int urldecode(uint8_t *d, uint8_t *s, uint16_t lend, uint16_t lens);
//int urlencode(uint8_t *d, uint8_t *s, uint16_t lend, uint16_t lens);
int htmlcode(uint8_t *d, uint8_t *s, uint16_t lend, uint16_t lens);
void print_hex_dump(uint8_t *buf, uint32_t len, uint8_t k);
// char* str_to_upper_case(char* text);
uint32_t str_array(uint8_t *s, uint32_t *buf, uint32_t max_buf);
uint32_t str_array_w(uint8_t *s, uint16_t *buf, uint32_t max_buf);
uint32_t str_array_b(uint8_t *s, uint8_t *buf, uint32_t max_buf);
char* word_to_lower_case(char* text);
int rom_xstrcmp(char * pd, const char * ps);
int rom_xstrcpy(char * pd, const char * ps);

void *zalloc(size_t xWantedSize);

#if CONFIG_DEBUG_LOG > 0
#define DEBUGSOO 2
#define os_printf(...) rtl_printf(__VA_ARGS__)
#else
#define DEBUGSOO 0
#define os_printf(...)
#endif


#endif /* _INCLUDE_WEB_UTILS_H_ */
