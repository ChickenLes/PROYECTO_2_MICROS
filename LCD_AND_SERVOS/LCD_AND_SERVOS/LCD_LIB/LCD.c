/*
 * CFile1.c
 *
 * Created: 4/28/2025 11:31:10 AM
 * Author: itzle
 */ 


#include "LCD.h"

#define DIRECCION 0x27  // Dirección común del PCF8574T

//INICIA LA COMUNICACION I2C
void I2C_START() {
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
}

//TERMINA LA COMUNICACION I2C
void I2C_Stop() {
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

//MODULO PARA ESCRIBIR 
void I2C_WRITE(uint8_t dato) {
	TWDR = dato;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
}

//MODULO PARA ENVIAR LOS DATOS AL LCD POR SUS DIRECCIONES
void LCD_SEND(uint8_t dato, uint8_t rs) {
	uint8_t nibble_alto = dato & 0xF0;
	uint8_t nibble_bajo = (dato << 4) & 0xF0;
	
	I2C_START();
	I2C_WRITE((DIRECCION<<1));
	I2C_WRITE(nibble_alto | rs | 0x04 | 0x08); // EN=1, BACKLIGHT=1
	I2C_WRITE(nibble_alto | rs | 0x08);        // EN=0
	I2C_WRITE(nibble_bajo | rs | 0x04 | 0x08); // EN=1
	I2C_WRITE(nibble_bajo | rs | 0x08);        // EN=0
	I2C_Stop();
	_delay_us(100);
}

//CONFIGURACION LCD
void LCD_CONF() {
	//INICIAR CONFIGURACION
	_delay_ms(50);
	LCD_SEND(0x30, 0); _delay_ms(5);
	LCD_SEND(0x30, 0);_delay_us(100);
	LCD_SEND(0x30, 0);
	LCD_SEND(0x20, 0); // Modo 4 bits
	
	LCD_SEND(0x28, 0); // 2 líneas, 5x8
	LCD_SEND(0x0C, 0); // Display ON
	LCD_SEND(0x01, 0); // Clear
	_delay_ms(2);
}

//ESCRIBIR EN EL LCD
void LCD_WRITE(char *texto) {
	while(*texto) {
		LCD_SEND(*texto, 1); //RS=1 DATOS
		texto++;
	}
}

void LCD_CURSOR(uint8_t fila) {
	LCD_SEND(0x80 | (fila * 0x40), 0); //0x80 LINEA 1, 0xC0 LINEA 2
}

void LCD_CLEAR() {
	LCD_SEND(0x01, 0);  // Envía el comando CLEAR (0x01) con RS=0
	_delay_ms(2);       // Espera el tiempo necesario para el comando
}