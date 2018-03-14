/*
 *  ADC + Websocket
 *
 *  Created on: 18.06.2017.
 *      Author: pvvx
 */

#include <platform_opts.h>

#include "device.h"
#include "PinNames.h"

#include "basic_types.h"
#include "diag.h"

#include "FreeRTOS.h"
#include "diag.h"

//------------------------------------------------------------------------------
#include "objects.h"
#include "PinNames.h"
#include "analogin_api.h"
#include "timer_api.h"

#include "web/web_utils.h"
#include "web/web_srv.h"
#include "web/websock.h"
#include "web/web_websocket.h"
#include "adc/adc_drv.h"

//------------------------------------------------------------------------------

#define USE_ADC_API 1
//------------------------------------------------------------------------------
#define ADC_DECIMATION_1		1
#define ADC_DECIMATION_2		2
#define ADC_DECIMATION_4		3
#define ADC_DECIMATION_8		4

#define ADC_SAMPLE_CLK_x1		0
#define ADC_SAMPLE_CLK_x2		1
#define ADC_SAMPLE_CLK_x4		2
#define ADC_SAMPLE_CLK_x8		3

#define ADC_DECIMATION_FILTER ADC_DECIMATION_1
#define ADC_SAMPLE_XCLK ADC_SAMPLE_CLK_x8
#define ADC_SMPS = 975*(1<<ADC_SAMPLE_XCLK)/(1<<ADC_DECIMATION_FILTER)
// 975*(1<<2)/(1<<1) = 1950 sps


//------------------------------------------------------------------------------
typedef struct _adc_data {
	uint16_t us0;
	uint16_t us1;
} ADC_DATA, *PADC_DATA;

typedef struct _adc_drv {
	    int8_t	init;			// флаг
		uint8_t	dcmf;			// ADC_DECIMATION_FILTER
		uint8_t xclk;			// ADC_SAMPLE_XCLK

		uint16_t count;			// счетчик считанных значений
		uint16_t overrun;		// счет переполнений буфера

		uint16_t buf_idx;		// объем буфера pbuf[buf_idx+1], максимальный индекс-номер замера
		uint16_t buf_rx;		// индекс-номер ещё не считанного замера
		uint16_t buf_tx;		// индекс-номер для записи следующего замера
		PADC_DATA pbuf;
#ifdef ADC_USE_TIMER
		gtimer_t timer;
#endif
} ADC_DRV, *PADC_DRV;

ADC_DRV adc_drv = {
		.dcmf = ADC_DECIMATION_FILTER,
		.xclk = ADC_SAMPLE_XCLK,
		.buf_idx = 709 // (1460*2 - 80)/(sizeof(ADC_DATA))	// циклический буфер на ~1420 замеров (см. sizeof(ADC_DATA))
								// Если шаг заполнения 1 ms -> буфер на 1.4 сек
								// Оптимизация под TCP: (TCP_MSS*2 - 80)/2 = (1460*2 - 80)/2 = 1420
};

void adc_int_handler(void *par) {
	union {
		uint16_t w[4];
		uint32_t d[2];
	}buf;

	PADC_DRV p = par; // &adc_drv
	ADC_TypeDef * adc = (ADC_TypeDef *)(ADC_REG_BASE);
	uint32_t adc_isr = adc->INTR_STS;
	buf.d[0] = adc->FIFO_READ; // ADC_Read();
	buf.d[1] = adc->FIFO_READ; // ADC_Read();
	if(p->pbuf) {
		PADC_DATA pd = p->pbuf + p->buf_tx;
		pd->us0 = buf.w[1];
		pd->us1 = buf.w[2];
		if(p->buf_tx >= p->buf_idx) p->buf_tx = 0;
		else p->buf_tx++;
		if(p->buf_rx == p->buf_tx) {
			p->overrun++;
			if(p->overrun == 0) p->init = 2; // overrun
			if(p->buf_rx >= p->buf_idx) p->buf_rx = 0;
			else p->buf_rx++;
		};
	};
	/* Clear ADC Status */
	adc->INTR_STS = adc_isr;
}

size_t adc_getdata(void *pd, uint16_t cnt)
{
	PADC_DRV p = &adc_drv;
	if(p->init <= 0) return 0;
	uint16_t *pus = (uint16_t *) pd;
	taskDISABLE_INTERRUPTS();
	uint16_t buf_rx = p->buf_rx;
	*pus++ = cnt;		// кол-во замеров
	*pus++ = p->count + p->overrun; // индекс замера для анализа пропусков на стороне приемника
	// если не пропущено, то равен прошлому + кол-во считанных замеров в прошлом блоке
	p->count += cnt; //	p->overrun = 0;
	uint8_t *puc = (uint8_t *) pus;
	if(cnt) {
		uint16_t lend = buf_rx + cnt;
		if(lend > p->buf_idx) {
			lend -= p->buf_idx + 1;
			p->buf_rx = lend;
		} else {
			p->buf_rx = lend;
			lend = 0;
		};
		size_t len = (cnt - lend) *sizeof(ADC_DATA);
		if(len)	memcpy(puc, (void *)(p->pbuf + buf_rx), len);
		if(lend) memcpy(puc + len, (void *)p->pbuf, lend *sizeof(ADC_DATA));
	}
	taskENABLE_INTERRUPTS();
	return cnt * sizeof(ADC_DATA) + 4;
}

uint16_t adc_chkdata(uint16_t cnt)
{
	PADC_DRV p = &adc_drv;
	if(p->init <= 0) return 0;
	int len = p->buf_tx - p->buf_rx;
	if(len < 0) len += p->buf_idx + 1;
	if(cnt > (uint16_t)len) cnt = (uint16_t)len;
	return cnt;
}

int adc_ws(TCP_SERV_CONN *ts_conn, uint8_t cmd)
{
	PADC_DRV p = &adc_drv;
	switch(cmd) {
	case 'd': // deinit
		if(p->init > 0) {
#ifdef ADC_USE_TIMER
		    gtimer_stop(&p->timer);
		    gtimer_deinit(&p->timer);
			ADCDisable();
#else
#if USE_ADC_API
			ADC_Cmd(DISABLE);
			ADC_INTClear();
			InterruptDis(ADC_IRQ);
#endif
#endif
#if USE_ADC_API
		    /* To release DAC delta sigma clock gating */
			PLL2_Set(BIT_SYS_SYSPLL_CK_ADC_EN, DISABLE);
			/* Turn off ADC active clock */
			RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, DISABLE);
#else
			ADCDeInit();
			ADCIrqDeInit();
#endif
			if(p->pbuf) {
				free(p->pbuf);
				p->pbuf = NULL;
			}
			p->init = -1;
			return 0;
		}
		return 1;
	case 'c': // get count
		return adc_chkdata(p->buf_idx + 1);
	case '1': // set ADC_DECIMATION_1
		p->dcmf = 1;
		return 0;
	case '2': // set ADC_DECIMATION_2
		p->dcmf = 2;
		return 0;
	case '4': // set ADC_DECIMATION_4
		p->dcmf = 3;
		return 0;
	case '8': // set ADC_DECIMATION_8
		p->dcmf = 4;
		return 0;
	case 'A': // set ADC_SAMPLE_XCLK
		p->xclk = 0;
		return 0;
	case 'B': // set ADC_SAMPLE_XCLK
		p->xclk = 1;
		return 0;
	case 'C': // set ADC_SAMPLE_XCLK
		p->xclk = 2;
		return 0;
	case 'D': // set ADC_SAMPLE_XCLK
		p->xclk = 3;
		return 0;
	case 'i': // Info init
		return p->init;
	default: // get_data
		if(p->init <= 0) {
			p->count = 0;
			p->overrun = 0;
//			p->errs = 0;
			if(!p->pbuf) {
				p->pbuf = zalloc((p->buf_idx + 1) * sizeof(ADC_DATA));
				if(!p->pbuf) {
					error_printf("Error create buffer!\n");
					return -1;
				};
				p->buf_tx = 0;
				p->buf_rx = 0;
			};
//			DiagPrintf("ADCInit...\n");
#if USE_ADC_API
			ADC_InitTypeDef ADCInitStruct;

			/* To release ADC delta sigma clock gating */
			PLL2_Set(BIT_SYS_SYSPLL_CK_ADC_EN, ENABLE);

			/* Turn on ADC active clock */
			RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

			ADC_InitStruct(&ADCInitStruct);
			ADCInitStruct.ADC_BurstSz = 4;
			ADCInitStruct.ADC_OneShotTD = 4; /* means 4 times */
			ADCInitStruct.ADC_OverWREn = ENABLE;
			ADC_AnaparAd[0] = 0x00953b10
					| BIT_CTRL_ADC_SAMPLE_CLKL(p->xclk);
			ADC_AnaparAd[1] = BIT_ADC_DIGITAL_RST_BAR
					| BIT_ADC_EXT_VREF_EN
					| BIT_ADC_DECIMATION_FILTER_ORDER
					| BIT_CTRL_DOWN_SAMPLE_RATE(p->dcmf);
			ADC_Init(&ADCInitStruct);
//			ADC_SetOneShot(DISABLE, 1, ADCInitStruct.ADC_OneShotTD);
			/* ADC Interrupt Initialization */
			ADC_INTClear();
			ADC_INTConfig(BIT_ADC_FIFO_FULL_EN | BIT_ADC_FIFO_RD_REQ_EN, ENABLE);
			InterruptRegister((IRQ_FUN)&adc_int_handler, ADC_IRQ, (u32)p, 5);
			InterruptEn(ADC_IRQ, 5);
			ADC_Cmd(ENABLE);
#else
			ADCInit(USE_ADC_CHL);
			ADCIrqInit(adc_int_handler,(uint32_t)p, BIT_ADC_FIFO_FULL_EN | BIT_ADC_FIFO_RD_REQ_EN);
			ADCEnable();
#endif
		    p->init = 1;
//			return 0;
		}
	case 'g': // get
		{
			uint32_t i = adc_chkdata(p->buf_idx + 1);
			if(i) {
				WEB_SRV_CONN *web_conn = (WEB_SRV_CONN *)ts_conn->linkd;
				i = mMIN((web_conn->msgbufsize / sizeof(ADC_DATA)), i);
				if(websock_tx_frame(ts_conn, WS_OPCODE_BINARY | WS_FRAGMENT_FIN, web_conn->msgbuf, adc_getdata(web_conn->msgbuf, i)) != ERR_OK)
					return -1;
			}
			return i;
		}
	}
	return -1;
}

