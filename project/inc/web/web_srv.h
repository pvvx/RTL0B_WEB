/*
 * File: web_srv.h
 * Description: The web server configration.
 * Small WEB server ESP8266EX
 * Author: PV` 
 */

#ifndef _INCLUDE_WEB_SRV_H_
#define _INCLUDE_WEB_SRV_H_

#include "web/tcp_srv_conn.h"
#ifdef WEBSOCKET_ENA
#include "websock.h"
#endif

#define WEB_SVERSION "0.2.0"
#define DEFAULT_WEB_PORT USE_WEB // 80
#define USE_WEB_AUTH_LEVEL 1

/****************************************************************************
  ***************************************************************************/

typedef enum
{
	WEB_AUTH_NONE = 0,
	WEB_AUTH_LEVEL_USER,
	WEB_AUTH_LEVEL_USER1,
	WEB_AUTH_LEVEL_WEBFS,
	WEB_AUTH_LEVEL_OTA,
	WEB_AUTH_LEVEL_SUPERVISOR
} WEB_AUTH_LEVEL_TYPE;

// File type definitions
typedef enum
{
        HTTP_TXT = 0,     // File is a text document
        HTTP_HTML,        // File is HTML (extension .htm)
        HTTP_CGI,         // File is HTML (extension .cgi)
        HTTP_XML,         // File is XML (extension .xml)
        HTTP_CSS,         // File is stylesheet (extension .css)
        HTTP_ICO,         // File is ICO  vnd.microsoft.icon
        HTTP_GIF,         // File is GIF image (extension .gif)
        HTTP_PNG,         // File is PNG image (extension .png)
        HTTP_JPG,         // File is JPG image (extension .jpg)
		HTTP_SVG,		  // File is SVG image (extension .svg)
        HTTP_JAVA,        // File is java (extension .js)
        HTTP_SWF,         // File is ShockWave-Flash (extension .swf)
        HTTP_WAV,         // File is audio (extension .wav)
        HTTP_PDF,         // File is PDF (extension .pdf)
        HTTP_ZIP,         // File is ZIP (extension .zip)
        HTTP_BIN,         // File is BIN (extension .bin)
        HTTP_UNKNOWN      // File type is unknown
} HTTP_FILE_TYPE;


// extended state data for each connection
#define FileNameSize 64
#define VarNameSize 64
#define CmdNameSize 32

typedef struct
{
    uint16_t httpStatus;        // Request method/status
	uint16_t uri_len;
	uint16_t head_len;
	uint16_t cookie_len;
    uint8_t pFilename[FileNameSize];
	uint8_t *puri; 		// указатель на строку с переменными запроса к файлу
	uint8_t *phead; 		// HTTP Headers
	uint8_t *pcookie; 	// cookie
	uint8_t *pcontent; 	// content
	uint32_t content_len; //
	uint8_t httpver; 		// версия HTTP клиента в BCD (0x00 = неизвестен; 0x09 = HTTP/0.9; 0x10 = HTTP/1.0; 0x11 = HTTP/1.1)
    uint8_t fileType;     // File type to return with Content-Type
} HTTP_CONN;


typedef void (* web_func_cb)(TCP_SERV_CONN *ts_conn);
typedef uint32_t (* web_ex_func_cb)(uint32_t flg); // внешняя или отложенная функция

typedef struct
{
	web_ex_func_cb fnc;
	void * param;
	uint16_t	pause_ms;
} WEB_SRV_QFNK;

typedef struct
{
	uint32_t webflag;		// флаги для http/web сервера
	uint8_t  bffiles[4];	// четыре Files pointers для оработки вложенных файлов include
	uint32_t udata_start;	// udata "start=0x..."
	uint32_t udata_stop;	// udata "stop=0x..."
	uint8_t  *msgbuf;		// указатель на текущий буфер вывода
	uint16_t msgbuflen;	// кол-во занятых байт в буфере msgbuf
	uint16_t msgbufsize;	// размер буфера
	web_func_cb func_web_cb; // calback функция у httpd для обработки приема/передачи кусками
	uint32_t content_len; // размер файла для передачи (GET/POST) или приема, если принимается внешний файл (POST + SCB_RXDATA)
#ifdef WEBSOCKET_ENA
	WS_FRSTAT ws;	// параметры websoc
#endif
#if USE_WEB_AUTH_LEVEL
	uint8_t auth_level;   // Уровень авторизации пользователя по паролю WEB_AUTH_LEVEL_TYPE
	uint8_t auth_realm; 	// Требуемый уровень авторизации (минимальный уровень) WEB_AUTH_LEVEL_TYPE
#endif
	uint8_t fileType;     // File type to return with Content-Type (if SCB_FCALBACK)
} WEB_SRV_CONN;

typedef enum
{
	WEBFS_MAX_HANDLE = 251,
	WEBFS_NODISK_HANDLE,
	WEBFS_WEBCGI_HANDLE,
	WEBFS_UPLOAD_HANDLE
} WEBFS_NUM_HANDLE;

// webflag:

#define  SCB_CLOSED		0x000001 // соединение закрыто
#define  SCB_DISCONNECT	0x000002 // выход на DISCONNECT
#define  SCB_FCLOSE		0x000004 // закрыть файлы
#define  SCB_FOPEN		0x000008 // файл(ы) открыт(ы)
#define  SCB_FCALBACK	0x000010 // file use ~calback~
#define  SCB_FGZIP		0x000020 // файл GZIP
#define  SCB_CHUNKED	0x000040 // передача шинковкой
#define  SCB_RETRYCB	0x000080 // вызвать повтор CalBack
#define  SCB_POST		0x000100 // POST
#define  SCB_GET		0x000200 // GET
#define  SCB_AUTH		0x000400 // необходима авторизация
#define  SCB_FINDCB		0x000800 // используется парсингом ~calback~
#define  SCB_RXDATA		0x001000 // прием данных (файла)
#define  SCB_HEAD_OK	0x002000 // заголовок HTTP принят и обработан
#define  SCB_BNDR		0x004000 // прилеплен Content-Type: multipart/form-data; boundary="..."
#define  SCB_REDIR		0x008000 // Redirect 302
#define  SCB_WEBSOC		0x010000 // WebSocket
#define  SCB_WSDATA		0x020000 // WebSocket data
#define  SCB_SYSSAVE	0x040000 // по закрытию соединения вызвать sys_write_cfg()


#define  SCB_OPEN       0

#define  SetSCB(a) web_conn->webflag |= a
#define  FreeSCB() web_conn->webflag = SCB_FREE
#define  SetNextFunSCB(a) web_conn->func_web_cb = a
#define  ClrSCB(a) web_conn->webflag &= ~(a)
#define  CheckSCB(a) (web_conn->webflag & (a))

#define  FreeSCB() web_conn->webflag = SCB_FREE
#define  OpenSCB() web_conn->webflag = SCB_OPEN

#define MAXLENBOUNDARY 64
typedef struct s_http_upload
{
  uint16_t status;
  uint16_t sizeboundary;
  uint8_t  boundary[MAXLENBOUNDARY+1];
  uint8_t  name[VarNameSize];
  uint8_t  filename[VarNameSize];
#ifdef USE_OVERLAY
  uint32_t segs; // кол-во сегментов оверлея // пока в web_conn->web_disc_par
  uint32_t start; // адрес запуска оверлея
#endif
  uint32_t fsize;
  uint32_t faddr;
  uint8_t *pbndr;
  uint8_t *pnext;
} HTTP_UPLOAD;

typedef struct s_http_response
{
  uint32_t status;
  uint32_t flag;
  const char * headers;
  const char * default_content;
} HTTP_RESPONSE;

// HTTP_RESPONSE.flags:
#define HTTP_RESP_FLG_END       0x8000
#define HTTP_RESP_FLG_NONE      0x0000
#define HTTP_RESP_FLG_FINDFILE  0x0001
#define HTTP_RESP_FLG_REDIRECT  0x0002

#define tcp_put(a) web_conn->msgbuf[web_conn->msgbuflen++] = a
#define tcp_htmlstrcpy(str, len) web_conn->msgbuflen += htmlcode(&web_conn->msgbuf[web_conn->msgbuflen], str, web_conn->msgbufsize - web_conn->msgbuflen - 1, len)
//#define tcp_urlstrcpy(str, len) web_conn->msgbuflen += urlencode(&web_conn->msgbuf[web_conn->msgbuflen], str, web_conn->msgbufsize - web_conn->msgbuflen - 1, len)
#define tcp_puts(...) web_conn->msgbuflen += rtl_sprintf((char *)&web_conn->msgbuf[web_conn->msgbuflen], __VA_ARGS__)
#define tcp_puts_fd(...) web_conn->msgbuflen += rtl_sprintf((char *)&web_conn->msgbuf[web_conn->msgbuflen], __VA_ARGS__)
#define tcp_put_mac(m) web_conn->msgbuflen += mactostr((char *)&web_conn->msgbuf[web_conn->msgbuflen], (unsigned char *)m, true)
/*
#define tcp_puts_fd(fmt, ...) do { \
		static const char flash_str[] ICACHE_RODATA_ATTR = fmt;	\
		web_conn->msgbuflen += rtl_sprintf((char *)&web_conn->msgbuf[web_conn->msgbuflen], (char *)flash_str, ##__VA_ARGS__); \
		} while(0)
*/
//#define tcp_strcpy(a) web_conn->msgbuflen += ets_strlen((char *)ets_strcpy((char *)&web_conn->msgbuf[web_conn->msgbuflen], (char *)a))
#define tcp_strcpy(a) web_conn->msgbuflen += rom_xstrcpy((char *)&web_conn->msgbuf[web_conn->msgbuflen], (const char *)a)
#define tcp_strcpy_fd(a) web_conn->msgbuflen += rom_xstrcpy((char *)&web_conn->msgbuf[web_conn->msgbuflen], (const char *)a)
/*
#define tcp_strcpy_fd(fmt) do { \
		static const char flash_str[] ICACHE_RODATA_ATTR = fmt;	\
		web_conn->msgbuflen += rom_xstrcpy((char *)&web_conn->msgbuf[web_conn->msgbuflen], (char *)flash_str); \
		} while(0)
*/
uint32_t ahextoul(uint8_t *s);
err_t webserver_init(uint16_t portn);
err_t webserver_close(uint16_t portn);
err_t webserver_reinit(uint16_t portn);

BaseType_t webserver_qfn(web_ex_func_cb fnc, void * param, uint16_t	pause_ms); // вызов функции из task с low priority

#endif /* _INCLUDE_WEB_SRV_H_ */
