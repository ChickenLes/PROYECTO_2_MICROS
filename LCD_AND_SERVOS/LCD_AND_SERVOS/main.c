/*
 * main.c
 *
 * Created: 4/10/2025 11:04:15 AM
 *  Author: itzle
 */ 
//-----------------------------------------------
// UNIVERSIDAD DEL VALLE DE GUATEMALA
// IE2023: PROGRAMACION DE MICROCONTROLADORES
// LABORATORIO4.C
// AUTOR: ANTHONY ALEJANDRO BOTEO LÓPEZ
// PROYECTO: PRELABORATORIO 4
// HARDWARE: ATMEGA328P
// CREADO: 04/10/2025
// ULTIMA MODIFICACION: 05/23/2025
// DESCRIPCIÓN:
//-----------------------------------------------


#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LCD_LIB/LCD.h"
#include "PWM_LIB/PWM.h"
#include "ADC_LIB/ADC_LIB.h"
#include "PIN_CHANGE_LIB/PIN_CHANGE.h"
#include "UART_LIB/UART_LIB.h"
#include "EEPROM_LIB/EEPROM_LIB.h"

//Modos de operación
#define MODO_EEPROM 0
#define MODO_MANUAL 1
#define MODO_UART   2

//Direcciones de guardado
#define EEPROM_SLOT0 0x0000  // 24 (12 posiciones x 2 bites)
#define EEPROM_SLOT1 0x0018  
#define EEPROM_SLOT2 0x0030
#define EEPROM_SLOT3 0x0048

//Variables globales
volatile uint8_t modo = MODO_EEPROM;  // Modo inicial

//Direcciones
volatile uint8_t slot_seleccionado = 0;
uint16_t valores_grabados[12];
uint16_t posiciones_servo[4][12];
uint16_t last_cadera = 0;
uint16_t last_codo = 0;
uint32_t last_send = 0;

//Tiempos
volatile uint8_t parpadeo = 0;
volatile uint16_t contador_ms = 0;
volatile uint8_t estado_grabacion = 0;
volatile uint16_t tiempo_restante = 0;
volatile uint16_t tiempo_inicio = 0;
#define TIEMPO_GRABACION 15000 // 15 segundos en ms
#define UART_TIMEOUT 5000

//Bandera
volatile uint8_t control_mode = 0;  // 0 = Manual, 1 = UART
volatile uint32_t last_uart_time = 0;
volatile uint8_t cambio_desde_uart = 0;



//Prototipos de funciones
void LCD_NUM_PSERVO(char *nombre_base, uint8_t numero_servo, uint8_t fila, uint16_t valor_adc);
void mostrar_modo_eeprom();
void mostrar_modo_manual();
void mostrar_modo_guardar();
void manejar_grabacion();
void cargar_posiciones(uint8_t slot);
void enviar_valores_servos();
void mostrar_modo_uart();
void procesar_comando_uart(const char* comando);
void TIMER0_CONF(void);

//Configuracion timer 0
void TIMER0_CONF(void) {
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01) | (1 << CS00);
    OCR0A = 249;
    TIMSK0 = (1 << OCIE0A);
}

//ISR timer 0
ISR(TIMER0_COMPA_vect) {
    static uint16_t contador_1ms = 0;
    static uint16_t contador_envio = 0;
    
    contador_1ms++;
    contador_envio++;
    
    if(contador_1ms >= 1000) {
        contador_1ms = 0;
        contador_ms += 1000;
    }
    
    if(contador_envio >= 500) {
        contador_envio = 0;
        enviar_valores_servos();
    }
    
    if(contador_1ms % 300 == 0) {
        parpadeo = !parpadeo;
    }
}

//ISR UART
ISR(USART_RX_vect) {
    static char buffer[64];
    static uint8_t idx = 0;
    char c = UDR0;
    
    if (c == '\n' || idx >= sizeof(buffer)-1) {
        buffer[idx] = '\0';
        procesar_comando_uart(buffer);
        idx = 0;
    } else if (c >= 32 && c <= 126) {
        buffer[idx++] = c;
    }
}

//Funcion para UART
void procesar_comando_uart(const char* comando) {
	if (strncmp(comando, "PC0", 3) == 0) {
		PUSH_FLAG = 1;
		cambio_desde_uart = 1;  
		return;
	}
	control_mode = 1;  //Cambiar modo UART
	last_uart_time = contador_ms;
	
	char temp[32];
	
	//Simulacion PC0
	if (strncmp(comando, "PC0", 3) == 0) {
		PUSH_FLAG = 1;
	}
	//Simulacion PC1
	else if (strncmp(comando, "PC1", 3) == 0) {
		PUSH_FLAG = 2;
	}
	//Simulacion SELECTOR DE CADERA
	else if (strncmp(comando, "CADERA_SEL:", 11) == 0) 
	{  
		uint8_t val = atoi(comando + 11); 
		if (val >= 1 && val <= 6) {
			sel_cadera = val-1;
			sprintf(temp, "OK_CAD:%d\n", val);
			UART_PrintString(temp);
		}
	}
	//Simulacion SELECTOR DE CODO
	else if (strncmp(comando, "CODO_SEL:", 9) == 0)
	{  
		uint8_t val = atoi(comando + 9);  
		if (val >= 1 && val <= 6) {
			sel_codo = val-1;
			sprintf(temp, "OK_COD:%d\n", val);
			UART_PrintString(temp);
		}
	}
	//Simulacion ANGULO CADERA
    else if (strncmp(comando, "ANG_CAD:", 8) == 0) {
        uint8_t angulo = atoi(comando + 8);
        if (angulo <= 100) {
            uint16_t valor_pwm = 500 + (angulo * 1000UL / 100);
            SET_SERVO_CADERA(valor_pwm);
            sprintf(temp, "CAD_SET:%d\n", angulo);
            UART_PrintString(temp);
        }
    }
	//Simulacion ANGULO CODO
    else if (strncmp(comando, "ANG_COD:", 8) == 0) {
        uint8_t angulo = atoi(comando + 8);
        if (angulo <= 100) {
            uint16_t valor_pwm = 500 + (angulo * 2000UL / 100);
            SET_SERVO_CODO(valor_pwm);
            sprintf(temp, "COD_SET:%d\n", angulo);
            UART_PrintString(temp);
        }
    }
}

//Envio de valores a servos
void enviar_valores_servos() {
    if (control_mode == 1) {
        uint16_t current_cadera = (sel_cadera * 100UL) / 6;
        uint16_t current_codo = (sel_codo * 100UL) / 6;
        
        if ((current_cadera != last_cadera || current_codo != last_codo) || 
            (contador_ms - last_send > 30000)) {
            
            char buffer[32];
            sprintf(buffer, "CAD:%d COD:%d\n", current_cadera, current_codo);
            UART_PrintString(buffer);
            
            last_cadera = current_cadera;
            last_codo = current_codo;
            last_send = contador_ms;
        }
    }
}


int main(void) {
    
    CONF_TIMER1();
    ADC_CONF();
    PIN_CHANGE_CONF();
    TIMER0_CONF();
    UART_CONF();
    
    //Configuracion LCD
    TWSR = 0x00;
    TWBR = 0x48;
    TWCR = (1 << TWEN);
    LCD_CONF();
    sei(); 

    while(1) {
        //Cambio de modo
        if (PUSH_FLAG == 1) {
	        PUSH_FLAG = 0;
	        
	        if (cambio_desde_uart) {
		        //Mantener bloqueo
		        control_mode = 1;
		        cambio_desde_uart = 0;
		        } else {
		        //Liberar bloqueo
		        control_mode = 0;
	        }
	        
	        modo = (modo + 1) % 3;
        }
        //Modo de operacion
        switch(modo) {
            case MODO_EEPROM: 
                mostrar_modo_eeprom(); 
                break;
                
            case MODO_MANUAL:
                if (estado_grabacion == 0) 
                    mostrar_modo_manual();
                else 
                    manejar_grabacion();
                break;
                
                
            case MODO_UART: 
                mostrar_modo_uart();
           
                break;
        }
    }
}

//-------------------------//
// FUNCIONES DE LOS MODOS  //
//-------------------------//

//modo uart
void mostrar_modo_uart() {
	LCD_CURSOR(0);
	LCD_WRITE("   MODO UART     ");
	LCD_CURSOR(1);
	LCD_WRITE("ONLINE");
}


//Modo EEPROM
void mostrar_modo_eeprom() {
	slot_seleccionado = (ADC_READ(2) * 3UL) / 1023;

	LCD_CURSOR(0);
	LCD_WRITE("MODO EEPROM     ");
	LCD_CURSOR(1);
	char linea[17];
	sprintf(linea, "Slot:%d PC1=CARGAR", slot_seleccionado + 1);
	LCD_WRITE(linea);

	if (PUSH_FLAG == 2) {
		PUSH_FLAG = 0;
		cargar_posiciones(slot_seleccionado);
		LCD_CLEAR();
		LCD_WRITE("POSICIONES CARGADAS");
		_delay_ms(1000);
	}
}

//Modo manual
void mostrar_modo_manual() {
	
	if (control_mode == 0) 
	{  
		sel_cadera = (ADC_READ(2) * 6UL) / 1024;
		sel_codo = (ADC_READ(3) * 6UL) / 1024;
	};
	
	
	uint16_t valor_cadera = ADC_READ(6);
	uint16_t valor_codo = ADC_READ(7);
	
	SET_SERVO_CADERA(500 + (valor_cadera * 1000UL / 1023));
	SET_SERVO_CODO(500 + (valor_codo * 1900UL / 1023)); 
	
	
	LCD_CURSOR(0);
	char linea1[17];
	snprintf(linea1, sizeof(linea1), "CAD %d: %lu%%", sel_cadera + 1, (valor_cadera * 100UL) / 1023);
	LCD_WRITE(linea1);

	LCD_CURSOR(1);
	char linea2[17];
	snprintf(linea2, sizeof(linea2), "COD %d: %lu%%", sel_codo + 1, (valor_codo * 100UL) / 1023);
	LCD_WRITE(linea2);

	if (PUSH_FLAG == 2) {
		PUSH_FLAG = 0;
		estado_grabacion = 1;
		LCD_CLEAR();
	}
}



void mostrar_modo_guardar() {
		
	uint16_t adc_valor = ADC_READ(2);
	slot_seleccionado = (adc_valor * 3UL) / 1023;
		
	
	LCD_CURSOR(0);
	LCD_WRITE("GRABAR EN SLOT:  ");
	LCD_CURSOR(1);
		
	char linea2[17];
	sprintf(linea2, "[%d] %ds         ",
	slot_seleccionado + 1,
	15 - (contador_ms / 1000));
		
	LCD_WRITE(linea2);

	
	LCD_WRITE(linea2);
	
	 if (PUSH_FLAG == 2) {
		 PUSH_FLAG = 0;
			 
		 // Guardar todas las posiciones actuales
		 for(uint8_t i = 0; i < 6; i++) {
			 SEL_SERVO(i, 0);
			 posiciones_servo[slot_seleccionado][i] = ADC_READ(6);
		 }
		 for(uint8_t i = 0; i < 6; i++) {
			 SEL_SERVO(0, i);
			 posiciones_servo[slot_seleccionado][i+6] = ADC_READ(7);
		 }
			 
			 
		 uint16_t eeprom_address;
		 switch(slot_seleccionado) {
			 case 0: eeprom_address = EEPROM_SLOT0; break;
			 case 1: eeprom_address = EEPROM_SLOT1; break;
			 case 2: eeprom_address = EEPROM_SLOT2; break;
			 case 3: eeprom_address = EEPROM_SLOT3; break;
			 default: return;
		 }
			 
		 escribir_eeprom(eeprom_address, valores_grabados, 12);
			 
		
		 LCD_CLEAR();
		 LCD_WRITE("¡SLOT GUARDADO! ");
		 _delay_ms(1000);
		 modo = 0;
	 }
	
		
	if (contador_ms >= 15000) {
		LCD_CLEAR();
		LCD_WRITE("GRABACION CANCEL.");
		_delay_ms(1000);
		modo = 0; // Volver al modo EEPROM
		contador_ms = 0;
	}
}

void manejar_grabacion() {
	switch(estado_grabacion) {
		case 1: { 
			slot_seleccionado = (ADC_READ(2) * 3UL) / 1023;
			
			LCD_CURSOR(0);
			LCD_WRITE("ELIGE SLOT (1-4):");
			LCD_CURSOR(1);
			char slot_msg[17];
			sprintf(slot_msg, "[%d] PC1=CONFIRMAR", slot_seleccionado + 1);
			LCD_WRITE(slot_msg);

			if (PUSH_FLAG == 2) {
				PUSH_FLAG = 0;
				estado_grabacion = 2;
				tiempo_inicio = contador_ms;
				LCD_CLEAR();
				LCD_WRITE("GRABANDO...     ");
				UART_PrintString("LCD:GRABANDO...");
			}
			break;
		}

		case 2: { 
			
			uint16_t tiempo_transcurrido = contador_ms - tiempo_inicio;
			uint16_t segundos_restantes = (TIEMPO_GRABACION/1000) - (tiempo_transcurrido/1000);
			
			uint8_t nueva_cadera = (ADC_READ(2) * 6UL) / 1024;
			uint8_t nuevo_codo = (ADC_READ(3) * 6UL) / 1024;
			
			if(nueva_cadera != sel_cadera || nuevo_codo != sel_codo) {
				SEL_SERVO(nueva_cadera, nuevo_codo);
			}

			
			uint16_t valor_cadera = ADC_READ(6);
			uint16_t valor_codo = ADC_READ(7);
			
			SET_SERVO_CADERA(500 + (valor_cadera * 1000UL / 1023));
			SET_SERVO_CODO(500 + (valor_codo * 2000UL / 1023));
			
			// Guardar valores actuales
			valores_grabados[sel_cadera] = valor_cadera;     
			valores_grabados[sel_codo + 6] = valor_codo;      

			// Mostrar en LCD
			LCD_CURSOR(0);
			char linea1[17];
			snprintf(linea1, sizeof(linea1), "CAD %d: %3d%% COD %d: %3d%%",
			sel_cadera + 1, (int)((valor_cadera * 100UL) / 1023),
			sel_codo + 1, (int)((valor_codo * 100UL) / 1023));
			LCD_WRITE(linea1);

			// Tiempo restante
			LCD_CURSOR(1);
			char tiempo_msg[17];
			sprintf(tiempo_msg, "T:%2us SLOT:%d    ", segundos_restantes, slot_seleccionado + 1);
			LCD_WRITE(tiempo_msg);

			// Control del tiempo
			if (tiempo_transcurrido >= TIEMPO_GRABACION) {
				uint16_t eeprom_address;
				switch(slot_seleccionado) {
					case 0: eeprom_address = EEPROM_SLOT0; break;
					case 1: eeprom_address = EEPROM_SLOT1; break;
					case 2: eeprom_address = EEPROM_SLOT2; break;
					case 3: eeprom_address = EEPROM_SLOT3; break;
					default: return;
				}
				
				
				LCD_CLEAR();
				LCD_WRITE("Guardando...");
				
				// Sirve para limpiar EERPROM ADDRESS
				for(uint8_t i = 0; i < 12; i++) {
					EEPROM_write(eeprom_address + i*2, 0xFF);
					EEPROM_write(eeprom_address + i*2 + 1, 0xFF);
					_delay_ms(10);
				}
				
		
				escribir_eeprom(eeprom_address, valores_grabados, 12);
				
			
				uint16_t datos_verificados[12];
				leer_eeprom(eeprom_address, datos_verificados, 12);
				
				LCD_CLEAR();
				uint8_t errores = 0;
				for(uint8_t i = 0; i < 12; i++) {
					if(datos_verificados[i] != valores_grabados[i]) {
						errores++;
					}
				}
				
				if(errores == 0) {
					LCD_WRITE("Guardado correcto!");
					} else {
					char msg[17];
					sprintf(msg, "Error: %d fallos", errores);
					LCD_WRITE(msg);
				}
				_delay_ms(2000);
				
				
				LCD_CLEAR();
				LCD_WRITE("GRABACION FINAL.");
				UART_PrintString("LCD:GRABACION FINAL.");
				_delay_ms(1000);
				estado_grabacion = 0;
				modo = 0;
				break;
			}

			// Cancelar
			if (PUSH_FLAG == 1) {
				PUSH_FLAG = 0;
				LCD_CLEAR();
				LCD_WRITE("GRABACION CANCEL.");
				UART_PrintString("LCD:GRABACION CANCEL.");
				_delay_ms(1000);
				estado_grabacion = 0;
				modo = 0;
			}
			break;
		}
	}
}

void cargar_posiciones(uint8_t slot) {
	uint16_t eeprom_address;
	switch(slot) {
		case 0: eeprom_address = EEPROM_SLOT0; break;
		case 1: eeprom_address = EEPROM_SLOT1; break;
		case 2: eeprom_address = EEPROM_SLOT2; break;
		case 3: eeprom_address = EEPROM_SLOT3; break;
		default:
		return;
	}
	
	LCD_CLEAR();
	LCD_WRITE("Cargando...");
	
	// Verificacion
	uint16_t datos_leidos[12];
	leer_eeprom(eeprom_address, datos_leidos, 12);
	
	// Validar datos
	for(uint8_t i = 0; i < 12; i++) {
		if(datos_leidos[i] > 1023) 
		{ 
			datos_leidos[i] = 512; // Valor seguro por defecto
		}
	}
	
	//Aplicar posiciones cadera
	for(uint8_t i = 0; i < 6; i++) {
		SEL_SERVO(i, 0);
		SET_SERVO_CADERA(500 + (datos_leidos[i] * 1000UL / 1023));
		_delay_ms(100);
	}
	//Aplicar posiciones codo
	for(uint8_t i = 0; i < 6; i++) {
		SEL_SERVO(0, i);
		SET_SERVO_CODO(500 + (datos_leidos[i+6] * 2000UL / 1023));
		_delay_ms(100);
	}
	
	LCD_CLEAR();
	LCD_WRITE("Posiciones cargadas");
	_delay_ms(1000);
}