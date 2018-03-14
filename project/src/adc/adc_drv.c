/*
 *  Simple ADC DRV (adc_drv.c)
 *  Created on: 18.06.2017.
 *  Author: pvvx
 */

#include <platform_opts.h>
#include "platform_autoconf.h"
#include "diag.h"
#include "device.h"

#define USE_ADC_API 0

/*--------------------------------------------------------------
void adc_int_handler(void *par) {
	ADC_TypeDef * adc = (ADC_TypeDef *)(ADC_REG_BASE);
	x =  adc->FIFO_READ; // Read ADC Data
	(void)adc->INTR_STS; // Clear ADC Status
}

	ADCInit(ADC2_SEL);
	ADCIrqInit(adc_int_handler,(uint32_t)p, BIT_ADC_FIFO_FULL_EN);
	ADCEnable();
	....
	ADCDisable();
	ADCIrqDeInit();
---------------------------------------------------------------*/

void ADCIrqInit(IRQ_FUN IrqFunc, uint32_t IrqData, uint32_t intr_enable) {
#if USE_ADC_API
	ADC_INTClear();
#else
	ADC_TypeDef * p = (ADC_TypeDef *)(ADC_REG_BASE);
	p->INTR_EN = 0;
#endif
	/* ADC Interrupt Initialization */
	InterruptRegister((IRQ_FUN)&IrqFunc, ADC_IRQ, (uint32_t)IrqData, 5);
	InterruptEn(ADC_IRQ, 5);
#if USE_ADC_API	
	ADC_INTConfig(BIT_ADC_FIFO_FULL_EN|BIT_ADC_FIFO_RD_REQ_EN, ENABLE);
#else
	p->INTR_EN = intr_enable; // HAL_ADC_WRITE32(REG_ADC_INTR_EN, intr_enable);
#endif	
}

void ADCIrqDeInit(void) {
	/* ADC Interrupt disable, poll mode will be used */
  	InterruptDis(ADC_IRQ);
//  	InterruptUnRegister(ADC_IRQ);
}

void ADCEnable(void) {
#if USE_ADC_API	
	ADC_Cmd(ENABLE);
#else
	ADC_TypeDef * p = (ADC_TypeDef *)(ADC_REG_BASE);
	/* Clear ADC Status */
	p->INTR_STS = p->INTR_STS; // HAL_ADC_READ32(REG_ADC_INTR_STS);
    /* ADC Enable */
//    p->POWER |= BIT_ADC_PWR_AUTO; // 0x04020b09; // (p->POWER & (~( BIT_ADC_PWR_AUTO | BIT_ADC_ISO_MANUAL))) | BIT_ADC_PWR12_MANUAL | BIT_ADC_PWR33_MANUAL;
//	p->ANAPAR_AD0 |= BIT_ADC_EN_MANUAL;
//	p->ANAPAR_AD1 |= BIT_ADC_DIGITAL_RST_BAR;
//    RTIM_Cmd(TIM3, ENABLE);
#endif
}

void ADCDisable(void) {
#if USE_ADC_API
	ADC_Cmd(DISABLE);
#else	
	ADC_TypeDef * p = (ADC_TypeDef *)(ADC_REG_BASE);
    p->INTR_EN = 0; // HAL_ADC_WRITE32(REG_ADC_INTR_EN, 0);
//    p->ANAPAR_AD0 &= ~BIT_ADC_EN_MANUAL; // 	HAL_ADC_WRITE32(REG_ADC_ANAPAR_AD0,	HAL_ADC_READ32(REG_ADC_ANAPAR_AD0) & (~BIT_ADC_EN_MANUAL));
//    p->ANAPAR_AD1 &= ~BIT_ADC_DIGITAL_RST_BAR; // 	HAL_ADC_WRITE32(REG_ADC_ANAPAR_AD1, HAL_ADC_READ32(REG_ADC_ANAPAR_AD1) & (~BIT_ADC_EN_MANUAL)); // BIT_ADC_DIGITAL_RST_BAR
    p->POWER &= ~BIT_ADC_PWR_AUTO; // HAL_ADC_WRITE32(REG_ADC_POWER, HAL_ADC_READ32(REG_ADC_POWER) & (~(BIT_ADC_PWR_AUTO)));
//    RTIM_Cmd(TIM3, DISABLE);
#endif    
}


void ADCInit(int adc_idx) {
	(void) adc_idx;
    /* ADC Function and Clock Enable */
	/* To release ADC delta sigma clock gating */
	PLL2_Set(BIT_SYS_SYSPLL_CK_ADC_EN, ENABLE);
	/* Turn on ADC active clock */
	RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);
#if USE_ADC_API
	ADC_InitTypeDef ADCInitStruct;
	/* Load ADC default value */
	ADC_InitStruct(&ADCInitStruct);
	ADCInitStruct.ADC_BurstSz = 8;
	ADCInitStruct.ADC_OverWREn = ENABLE;
	ADCInitStruct.ADC_OneShotTD = 4;
	ADC_AnaparAd[1] = BIT_ADC_DECIMATION_FILTER_ORDER | BIT_ADC_EXT_VREF_EN | BIT_CTRL_DOWN_SAMPLE_RATE(ADC_SAMPLE_CLK_2_12P500M); // 0xC1004;
	ADC_Init(&ADCInitStruct);
#else
	ADC_TypeDef * p = (ADC_TypeDef *)(ADC_REG_BASE);
	/* ADC Control register set-up*/
	p->CONTROL = BIT_ADC_OVERWRITE | BIT_CTRL_ADC_BURST_SIZE(4) | BIT_CTRL_ADC_THRESHOLD(0) | BIT_CTRL_ADC_DBG_SEL(0); // 	0x00080404
    /* ADC audio mode set-up */
    /* ADC enable manually setting */
	p->ANAPAR_AD0 = 0x0095fb12; // (p->ANAPAR_AD0 | BIT_ADC_AUDIO_EN | BIT_ADC_EN_MANUAL | BIT_ADC_SAMPLE_CLK) & (~(BIT_ADC_CHOP_EN | BIT_ADC_DEM_EN)); // ADC audio mode enable, ADC enable manually, BIT_ADC_SAMPLE_CLK & 1  ?
    /* ADC analog parameter 1 */
    p->ANAPAR_AD1 = 0x00101005; // BIT_ADC_DIGITAL_RST_BAR | BIT_ADC_EXT_VREF_EN | BIT_ADC_DECIMATION_FILTER_ORDER | BIT_CTRL_DOWN_SAMPLE_RATE(ADC_SAMPLE_CLK_2_12P500M); // 0x00101005
    /* ADC analog parameter 2 */
    p->ANAPAR_AD2 = 0x67884400;
    /* ADC analog parameter 3 */
    p->ANAPAR_AD3 = 0x77780039;
    /* ADC analog parameter 4 */
    p->ANAPAR_AD4 = 0x0004d501;
    /* ADC analog parameter 5 */
    p->ANAPAR_AD5 = 0x1E010800;
	/* ADC Power */
	p->POWER = BIT_ADC_ISO_MANUAL | BIT_CTRL_ADC_PWR_CUT_CNTR(2); // 0xr0020r08
#endif
}

void ADCDeInit(void) {
#if USE_ADC_API
	/* disable ADC*/
	ADC_Cmd(DISABLE);
	/* Clear ADC Status */
	ADC_INTClear();
#else
	ADCDisable();
#endif
	/* To release DAC delta sigma clock gating */
	PLL2_Set(BIT_SYS_SYSPLL_CK_ADC_EN, DISABLE);
	/* Turn off ADC active clock */
	RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, DISABLE);
}

