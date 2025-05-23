# -*- coding: utf-8 -*-
"""
Control para Adafruit IO 
"""
import serial
import time
from Adafruit_IO import MQTTClient

# Configuración
ADAFRUIT_USER = "//usuario"
ADAFRUIT_KEY = "//key"
SERIAL_PORT = 'COM4'
BAUDRATE = 9600
last_received = time.time()
# Inicializar conexión serial
arduino = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
time.sleep(2)

def send_command(cmd):
    """Envía comandos al Arduino"""
    arduino.write(f"{cmd}\n".encode())
    print(f"Enviado: {cmd}")

def handle_message(client, feed_id, payload):
    global last_received
    last_received = time.time()
    """Maneja mensajes de Adafruit IO"""
    feed = feed_id.split('/')[-1]
    
    if feed == 'CODO_SEL' and payload.isdigit():
        val = int(payload)
        if 1 <= val <= 6:
            send_command(f"CODO_SEL:{val}")
    elif feed == 'CADERA_SEL' and payload.isdigit():
        val = int(payload)
        if 1 <= val <= 6:
            send_command(f"CADERA_SEL:{val}")
    elif feed == 'ANGULO_CAD' and payload.isdigit():
        val = int(payload)
        if 0 <= val <= 100:
            send_command(f"ANG_CAD:{val}")
    elif feed == 'ANGULO_CODO' and payload.isdigit():
        val = int(payload)
        if 0 <= val <= 100:
            send_command(f"ANG_COD:{val}")
    elif feed == 'PC0' and payload == '0':
        send_command("PC0")
    elif feed == 'PC1' and payload == '0':
        send_command("PC1")

# Configurar Adafruit IO
aio = MQTTClient(ADAFRUIT_USER, ADAFRUIT_KEY)
aio.on_message = handle_message

# Conectar y suscribirse
aio.connect()
aio.subscribe('CODO_SEL')
aio.subscribe('CADERA_SEL')
aio.subscribe('ANGULO_CAD')
aio.subscribe('ANGULO_CODO')
aio.subscribe('PC0')
aio.subscribe('PC1')    
aio.loop_background()

print("Sistema listo. Esperando datos...")
while True:
    if arduino.in_waiting:
        line = arduino.readline().decode().strip()
        current_time = time.time()
        
        # Solo procesar si ha pasado más de 1 segundo desde el último envío
        if current_time - last_received > 1.0:
            if line.startswith("CAD:"):
                parts = line.split()
                cadera = parts[0].split(':')[1]
                codo = parts[1].split(':')[1]
                aio.publish('CADERA', cadera)
                aio.publish('CODOS', codo)
                last_received = current_time
try:
    while True:
        if arduino.in_waiting:
            line = arduino.readline().decode().strip()
            if line:
                print(f"Recibido: {line}")
                
                if line.startswith("CADERA:"):
                    parts = line.split()
                    cadera = parts[0].split(':')[1]
                    codo = parts[1].split(':')[1]
                    aio.publish('CADERA', cadera)
                    aio.publish('CODOS', codo)
                elif "CMD:" in line:
                    print(f"Comando procesado: {line}")
                elif "CADERA_SEL:" in line:
                    print(f"Selección cadera actualizada: {line.split(':')[1]}")
                elif "CODO_SEL:" in line:
                    print(f"Selección codo actualizada: {line.split(':')[1]}")
        
        time.sleep(0.1)

except KeyboardInterrupt:
    print("\nCerrando conexiones...")
    arduino.close()
    aio.disconnect()