	/*
 * PWM.h
 *
 * Created: 4/26/2025 10:59:15 PM
 *  Author: itzle
 */ 


// PWM.h
#ifndef PWM_H_
#define PWM_H_

#include <avr/io.h>
#include <stdint.h>

//////////////////PINES SERVOS///////////////
//CADERA
#define CADERA_PORT PORTD
#define CADERA_DDR  DDRD
#define CADERA_MASK 0xFC  //PD2-PD7

//CODO
#define CODO_PORT   PORTB
#define CODO_DDR    DDRB
#define CODO_MASK   0x3F  //PB0-PB5

//MODULOS
void CONF_TIMER1(void);
void SET_SERVO_CADERA(uint16_t ancho_pulso);
void SET_SERVO_CODO(uint16_t ancho_pulso);
void SEL_SERVO(uint8_t cadera_num, uint8_t codo_num);

//VARIABLES GLOBALES
extern volatile uint16_t ancho_cadera;
extern volatile uint16_t ancho_codo;
extern volatile uint8_t sel_cadera;
extern volatile uint8_t sel_codo;
extern volatile uint8_t BANDERA_PASO;

#endif /* PWM_H_ */