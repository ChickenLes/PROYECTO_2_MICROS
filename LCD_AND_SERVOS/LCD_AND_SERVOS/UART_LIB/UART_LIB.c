/*
 * UART_LIB.c
 *
 * Created: 5/10/2025 12:50:30 PM
 *  Author: itzle
 */ 

#include "UART_LIB.h"
#include <stdlib.h>  

void UART_CONF(){
	UBRR0H = 0;
	UBRR0L = 103;  // 9600 bauds @ 16MHz
	
	UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);  // Habilitar interrupción RX
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  // 8 bits, sin paridad, 1 stop bit
}

uint8_t UART_available() {
	return (UCSR0A & (1 << RXC0));
}

void UART_TRANS(uint8_t DATOS){
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = DATOS;
}

uint8_t UART_RES(){
	while (!UART_available());  // Usar la nueva función
	return UDR0;
}

void UART_PrintString(const char *str) {
	while (*str) {
		UART_TRANS(*str++);
	}
	UART_TRANS('\r');  // Retorno de carro adicional
	UART_TRANS('\n');  // Salto de línea
}

void leer_comandos_uart() {
	static char buffer[64];
	static uint8_t idx = 0;
	
	while (UART_available()) {
		char c = UDR0;
		
		if (c == '\n' || c == '\r') {
			if (idx > 0) {
				buffer[idx] = '\0';
				procesar_comando_uart(buffer);
				idx = 0;
			}
			} else if (idx < sizeof(buffer) - 1) {
			buffer[idx++] = c;
		}
	}
}

// En UART_LIB.c
void UART_PrintNumber(uint16_t num) {
	char buffer[6]; // Suficiente para números hasta 65535
	itoa(num, buffer, 10);
	UART_PrintString(buffer);
}