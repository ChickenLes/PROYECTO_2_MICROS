/*
 * PIN_CHANGE.c
 *
 * Created: 8/05/2025 14:40:18
 *  Author: Bren
 */ 

#include "PIN_CHANGE.h"
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile uint8_t MODO_LCD = 0;
volatile uint16_t VALOR_JOYSTICK = 0;
volatile uint8_t PUSH_FLAG = 0;

//-------------------------------------------------
// CONFIGURACIÓN DE INTERRUPCIONES
//-------------------------------------------------
void PIN_CHANGE_CONF(void) {
	DDRC &= ~((1 << PC0) | (1 << PC1));
	PORTC |= (1 << PC0) | (1 << PC1);
	PCMSK1 |= (1 << PCINT8) | (1 << PCINT9);
	PCICR |= (1 << PCIE1);
	sei();
}

//-------------------------------------------------
// RUTINA DE INTERRUPCIÓN MEJORADA
//-------------------------------------------------
ISR(PCINT1_vect) {
	static uint8_t estado_anterior = 0xFF;
	uint8_t estado_actual = PINC & ((1 << PC0) | (1 << PC1));
	
	if (estado_actual != estado_anterior) {
		_delay_ms(20);  // Debounce físico
		estado_anterior = estado_actual;
		
		if (!(PINC & (1 << PC0))) PUSH_FLAG = 1;
		if (!(PINC & (1 << PC1))) PUSH_FLAG = 2;
	}
}