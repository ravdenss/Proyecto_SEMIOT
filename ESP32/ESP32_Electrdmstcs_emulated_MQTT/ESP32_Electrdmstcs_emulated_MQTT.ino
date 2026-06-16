//Valores dataset
float dataset_AP[] = {0.01,
0.01,
0.01,
0.01,
0,
0,
0.01,
0.01,
0,
0.01,
0,
0.01,
0.01,
0.01,
0.01,
0.01,
0.01,
0,
0.01,
0.01,
0,
0.01,
0.01,
0.01};
float dataset_hervidor[] ={ 
  0,
0,
0,
0,
0,
0,
0,
0.13,
0.13,
0,
0.1,
0,
0.15,
0,
0.08,
0,
0.11,
0.09,
0.19,
0.04,
0,
0,
0.1,
0
};
int max_dataset = 24;
int current = 0;

//Includes
#include <WiFi.h>
#include <PubSubClient.h>
#include "EmonLib.h"

//Definir tenasa
EnergyMonitor SCT013;
int pinSCT = 34;

// Replace the next variables with your SSID/Password combination
const char* ssid = "B323";
const char* password = "b323telem";

//const char* ssid = "kunga_nueva";
//const char* password = "filomena123.,";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "192.168.50.200";
//const char* mqtt_server = "192.168.1.97";


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  SCT013.current(pinSCT, 60.606); //for sensor 100A:50mA (ratio 2000:1 at 33Ohm Burden resistor -> 2000/33 = 60.606)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  //Medir muchas veces para limpiar cache
  double Irms;
  for (int i =0; i<100;i++){

    digitalWrite(LED_BUILTIN, HIGH);
    Irms = SCT013.calcIrms(1480);
    digitalWrite(LED_BUILTIN, LOW); 
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
                    // 1 min
  if (now - lastMsg > 60*1000) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);

    int test = analogRead(pinSCT);
    Serial.print("ADC= ");
    Serial.println(test);



    double Irms = SCT013.calcIrms(1480*10); 
    //double I_calib = Irms*1.0548-0.0142;  //use calib factor here
    double I_calib = Irms;
    float power = I_calib * 220;

    Serial.print("Current = ");
    Serial.print(Irms);
    Serial.println(" A");
    Serial.print("Power = ");
    Serial.print(power);
    Serial.println(" W");
    Serial.println("...");

    lastMsg = now;
    current = (current+1)%max_dataset;
    
    // Convert the value to a char array
    char tempString[8];
    dtostrf(dataset_AP[current], 1, 2, tempString);
    client.publish("Consumo_AP", tempString);
    dtostrf(dataset_hervidor[current], 1, 2, tempString);
    client.publish("Consumo_hervidor", tempString);
    //dtostrf(power, 1, 2, tempString);
    //client.publish("SCT-013", tempString);
    //Serial.println(dataset_AP[current]);
    //Serial.println(dataset_hervidor[current]);
  }
}
