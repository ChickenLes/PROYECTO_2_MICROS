/*
 * ADC_LIB.h
 *
 * Created: 5/1/2025 9:32:42 PM
 *  Author: itzle
 */ 


#ifndef ADC_LIB_H_
#define ADC_LIB_H_

#include <avr/io.h>
#include <stdint.h>

void ADC_CONF();
uint16_t ADC_READ(uint8_t canal);

#endif /* ADC_LIB_H_ */