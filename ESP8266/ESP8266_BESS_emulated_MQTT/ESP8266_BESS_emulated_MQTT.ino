#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <Wire.h>
#include <SPI.h>

#define LED_PIN 2 // Use BUILTIN_LED if you want to use the onboard LED


float dataset[] = {
0.004066589158,
0.004083363329,
0.004049959166,
0.004096279177,
0.004074976389,
0.004141675836,
0.004195901663,
0.004366723621,
0.004370793883,
0.004445965563,
0.00439160888,
0.004437527218,
0.004470855282,
0.004666662223,
0.003187548882,
0.005682693043,
0.03360042081,
0.1618720751,
0.07953764619,
0.001045246102,
0.001012548889,
0.001266866383,
0.001350101111,
0.001528895561,
0.002008782223,
0.001716886942,
0.001779386956,
0.03228064708,
0.04502863171,
0.001591993888,
0.001463094439,
0.00142052889,
0.0008164266671,
0.0005999302771,
0.00030835639,
0.00191254139,
0.003766765,
0.004362446664,
0.004424996119,
0.004500034444,
0.004474984726,
0.004462508342,
0.004487507226,
0.004487545561,
0.004508292224,
0.004495783623,
0.00444579889,
0.004462505273,
};

float dataset2[] = {
98,
98,
97.7,
97,
97,
97,
97,
97,
97,
97,
97,
97,
96.41666667,
96,
96,
96,
96,
96,
97.9,
100,
100,
100,
100,
100,
100,
100,
100,
99.8,
99.71666667,
100,
100,
100,
100,
100,
100,
99.73333333,
99,
99,
99,
99,
99,
99,
99,
99,
98.78333333,
98,
98,
98
};



int dataset_size = 48;
int current = 0;

#define WIFI_SSID "B323"
#define WIFI_PASSWORD "b323telem"

// Raspberry Pi Mosquitto MQTT Broker
#define MQTT_HOST IPAddress(192, 168, 50, 200)
#define MQTT_PORT 1883

// MQTT Topics
#define MQTT_TOPIC_EV_CHARGE "Carga_BESS"
#define MQTT_TOPIC_EV_SOC "SOC_BESS"

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 30*1000;        // Interval at which to publish sensor readings

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

/*void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}*/

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  pinMode(LED_PIN, OUTPUT); 
  digitalWrite(LED_PIN, LOW);

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  //mqttClient.onSubscribe(onMqttSubscribe);
  //mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  // If your broker requires authentication (username and password), set them below
  //mqttClient.setCredentials("REPlACE_WITH_YOUR_USER", "REPLACE_WITH_YOUR_PASSWORD");
  connectToWifi();
}

void loop() {
  unsigned long currentMillis = millis();
  // Every X number of seconds (interval = 10 seconds) 
  // it publishes a new MQTT message
  if (currentMillis - previousMillis >= interval) {
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    
    current = (current + 1)%dataset_size;
    // Save the last time a new reading was published
    previousMillis = currentMillis;
    
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_TOPIC_EV_CHARGE, 1, true, String(dataset[current]).c_str());
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_TOPIC_EV_CHARGE, packetIdPub1);
    //Serial.printf("Message: %.2f \n", temperature);

    uint16_t packetIdPub2 = mqttClient.publish(MQTT_TOPIC_EV_SOC, 1, true, String(dataset2[current]).c_str());
    Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_TOPIC_EV_SOC, packetIdPub2);
    //Serial.printf("Message: %.2f \n", humidity);
  }
}
