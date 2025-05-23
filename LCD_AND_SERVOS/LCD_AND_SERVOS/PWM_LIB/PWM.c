/*
 * CFile1.c
 *
 * Created: 4/26/2025 10:59:15 PM
 * Author: itzle
 */ 

#include "PWM.h"
#include <avr/interrupt.h>

// Variables globales
volatile uint16_t ancho_cadera = 1500;
volatile uint16_t ancho_codo = 1500;
volatile uint8_t sel_cadera = 0;
volatile uint8_t sel_codo = 0;
volatile uint8_t BANDERA_PASO = 0;

void CONF_TIMER1(void) {
	//CONFIGURACION PUERTOS
	CADERA_DDR |= CADERA_MASK; //MASCARA DE 0xFC
	CODO_DDR |= CODO_MASK;	//MASCARA DE 0x3F
	
	//CONFIGURACION CTC - TIMER1
	TCCR1A = 0; 
	TCCR1B = (1 << WGM12) | (1 << CS11); //PRESCALER 8
	OCR1A = 40000; 
	TIMSK1 = (1 << OCIE1A); //INTERRUPCION POR COMPARACIÓN
}

void SET_SERVO_CADERA(uint16_t ancho_pulso) {
	ancho_cadera = ancho_pulso;
}

void SET_SERVO_CODO(uint16_t ancho_pulso) {
	ancho_codo = ancho_pulso;
}

void SEL_SERVO(uint8_t cadera_num, uint8_t codo_num) {
	sel_cadera = cadera_num;
	sel_codo = codo_num;
}

// Interrupción para PWM por software
ISR(TIMER1_COMPA_vect) {
	static uint8_t ciclo = 0;
	
	switch(ciclo) {
		case 0:
		// Activar servo de cadera seleccionado
		CADERA_PORT &= ~CADERA_MASK;
		CADERA_PORT |= (1 << (sel_cadera + 2));
		OCR1A = ancho_cadera * 2;
		ciclo = 1;
		break;
		
		case 1:
		// Activar servo de codo seleccionado
		CADERA_PORT &= ~CADERA_MASK;
		CODO_PORT |= (1 << sel_codo);
		OCR1A = ancho_codo * 2;
		ciclo = 2;
		break;
		
		case 2:
		// Espera hasta el siguiente ciclo
		CODO_PORT &= ~CODO_MASK;
		OCR1A = 40000 - (ancho_cadera + ancho_codo) * 2;
		ciclo = 0;
		break;
	}
}