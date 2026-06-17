# Rui Santos & Sara Santos - Random Nerd Tutorials
# Complete project details at https://RandomNerdTutorials.com/raspberry-pi-pico-w-mqtt-micropython/

#from machine import Pin, I2C
from time import sleep
import network
import sys
from umqtt.simple import MQTTClient
import config
import machine
#import BME280

# Constants for MQTT Topics
MQTT_TOPIC_EV_CHARGE = 'Carga_EV'
MQTT_TOPIC_EV_SOC = 'SOC_EV'

# luz led
led = machine.Pin("LED", machine.Pin.OUT)

# MQTT Parameters
MQTT_SERVER = config.mqtt_server
MQTT_PORT = 0
MQTT_USER = config.mqtt_username
MQTT_PASSWORD = config.mqtt_password
MQTT_CLIENT_ID = b"raspberrypi_picow"
MQTT_KEEPALIVE = 7200
MQTT_SSL = False   # set to False if using local Mosquitto MQTT broker
MQTT_SSL_PARAMS = {'server_hostname': MQTT_SERVER}

# Nombre de tu archivo CSV subido a la Pico
CSV_FILENAME = 'EV_regularizado.csv'

def initialize_wifi(ssid, password):
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)

    # Connect to the network
    wlan.connect(ssid, password)

    # Wait for Wi-Fi connection
    connection_timeout = 10
    while connection_timeout > 0:
        if wlan.status() >= 3:
            break
        connection_timeout -= 1
        print('Waiting for Wi-Fi connection...')
        sleep(1)

    # Check if connection is successful
    if wlan.status() != 3:
        return False
    else:
        print('Connection successful!')
        network_info = wlan.ifconfig()
        print('IP address:', network_info[0])
        return True

def connect_mqtt():
    try:
        client = MQTTClient(client_id=MQTT_CLIENT_ID,
                            server=MQTT_SERVER,
                            port=MQTT_PORT,
                            user=MQTT_USER,
                            password=MQTT_PASSWORD,
                            keepalive=MQTT_KEEPALIVE,
                            ssl=MQTT_SSL,
                            ssl_params=MQTT_SSL_PARAMS)
        client.connect()
        return client
    except Exception as e:
        print('Error connecting to MQTT:', e)
        raise

def publish_mqtt(topic, value):
    client.publish(topic, value)
    print(topic)
    print(value)
    print("Publish Done")

try:
    if not initialize_wifi(config.wifi_ssid, config.wifi_password):
        print('Error connecting to the network... exiting program')
    else:
        # Connect to MQTT broker, start MQTT client
        client = connect_mqtt()
        
        # Abrimos el archivo CSV en modo lectura
        with open(CSV_FILENAME, 'r', encoding='utf-8') as file:
            # Leemos y descartamos la primera línea si tu CSV tiene encabezados 
            encabezado = file.readline() 
            
            while True:
                # Leer la siguiente línea del archivo
                line = file.readline()
                
                # Si la línea está vacía, llegamos al final del archivo CSV
                if not line:
                    print("Fin del archivo CSV alcanzado. Reiniciando ciclo...")
                    file.seek(0)           # Volver al inicio del archivo
                    file.readline()        # Saltar el encabezado de nuevo
                    continue               # Volver al inicio del bucle while
                
                # Limpiar saltos de línea (\n) y separar los valores por coma
                valores = line.strip().split(',')
                
                try:
                    # --- CONFIGURA TUS COLUMNAS AQUÍ ---
                    EV_Charge = valores[2] 
                    EV_Soc = valores[4]    
                    
                    # 1. ENCENDER LED justo antes de enviar
                    led.on()
                    
                    #print("Enviando datos...")
                    
                    # Publicar los valores leídos
                    publish_mqtt(MQTT_TOPIC_EV_CHARGE, EV_Charge)
                    publish_mqtt(MQTT_TOPIC_EV_SOC, EV_Soc)
                    
                    sleep(15)
                    # APAGAR LED inmediatamente después de enviar
                    led.off()
                    print("Datos enviados")
                    
                except IndexError:
                    print("Error: La línea leída no tiene suficientes columnas:", line)
                
                # Delay de 30 segundos antes de leer la siguiente fila del CSV
                # El LED permanecerá APAGADO durante estos 30 segundos
                sleep(15)

except Exception as e:
    print('Error fatal detectado:')
    sys.print_exception(e)