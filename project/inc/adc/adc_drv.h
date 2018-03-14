/*
 * Simple ADC DRV (adc_drv.h)
 *
 *  Created on: 19 июн. 2017 г.
 *      Author: pvvx
 */

#ifndef _DRIVER_ADC_DRV_H_
#define _DRIVER_ADC_DRV_H_

/* intr_enable = bits: REG_ADC_INTR_EN : 
BIT_ADC_FIFO_RD_ERROR_EN | BIT_ADC_FIFO_RD_REQ_EN | BIT_ADC_FIFO_FULL_EN ... */
void ADCIrqInit(IRQ_FUN IrqFunc, uint32_t IrqData, uint32_t intr_enable); 
void ADCIrqDeInit(void);

void ADCInit(int adc_idx); // adc_idx = ADC2_SEL = 2 ...
void ADCDeInit(void);   // ADC Deinit
void ADCEnable(void);	// ADC Start
void ADCDisable(void);	// ADC Stop


#endif /* _DRIVER_ADC_DRV_H_ */
