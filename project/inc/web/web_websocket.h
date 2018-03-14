#ifndef _WEB_WEBSOCKET_H_
/******************************************************************************
 * FileName: web_websocket.h
 * Description: websocket for web ESP8266
 * Author: PV`
 * (c) PV` 2016
*******************************************************************************/
#define _WEB_WEBSOCKET_H_
#include "user_config.h"
#ifdef WEBSOCKET_ENA
#include "websock.h"

err_t websock_tx_close_err(TCP_SERV_CONN *ts_conn, uint32_t err);
bool websock_rx_data(TCP_SERV_CONN *ts_conn);
err_t websock_tx_frame(TCP_SERV_CONN *ts_conn, uint32_t opcode, uint8_t *raw_data, uint32_t raw_len);

#endif // WEBSOCKET_ENA
#endif /* _WEB_WEBSOCKET_H_ */
