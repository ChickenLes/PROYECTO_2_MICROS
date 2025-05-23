/*
 * UART_LIB.h
 *
 * Created: 5/10/2025 12:34:01 PM
 *  Author: itzle
 */ 


#ifndef UART_LIB_H_
#define UART_LIB_H_

#include <avr/io.h>

void procesar_comando_uart(const char* comando);

void UART_CONF(void);
void UART_TRANS(uint8_t DATOS);
uint8_t UART_RES(void);
void UART_PrintString(const char *str);
void leer_comandos_uart(void);
uint8_t UART_available(void);
void UART_PrintNumber(uint16_t num);

#endif /* UART_LIB_H_ */