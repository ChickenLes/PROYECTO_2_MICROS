/*
 * ADC_LIB.c
 *
 * Created: 4/29/2025 8:33:31 PM
 *  Author: itzle
 */ 


#include "ADC_LIB.h"

//CONFIGURACION
void ADC_CONF() {
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
}

//LEER EL CANAL POR EL QUE PASAMOS
uint16_t ADC_READ(uint8_t canal) {
	ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADC;
}

