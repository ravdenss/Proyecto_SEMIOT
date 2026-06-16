#include "EmonLib.h"

EnergyMonitor SCT013;
int pinSCT = A0; //sensor pin is connected to A0

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#define ON_Board_LED 2  // On board LED, indicator when connecting to a wifi router

const char* ssid = "NguyenTuong";   // Your wifi name
const char* password = "064897435"; // Your wifi password

//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------

WiFiClientSecure client; // Create a WiFiClientSecure object

// Google spreadsheet script ID
String GAS_ID = "AKfycbxrlNIafnFUnfPritGOhXVPI4RGMAoY27WdC01AP0vtUhWYzHl9E4EYluJi8sALZ0iF";

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password); // Connect to your WiFi router
  Serial.println("");
    
  pinMode(ON_Board_LED,OUTPUT);     // On board LED port as output
  digitalWrite(ON_Board_LED, HIGH); // Turn off Led on board

  //SCT013.current(pinSCT, 60.606); //for sensor 100A:50mA (ratio 2000:1 at 33Ohm Burden resistor -> 2000/33 = 60.606)
  SCT013.current(pinSCT, 20);       //for sensor 20A/1V

  //----------------------------------------Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //----------------------------------------Make LED flashing when connecting to the wifi router
    digitalWrite(ON_Board_LED, LOW);
    delay(200);
    digitalWrite(ON_Board_LED, HIGH);
    delay(200);
    //----------------------------------------
  }
  //----------------------------------------
  digitalWrite(ON_Board_LED, HIGH); // Turn off the LED when it is connected to the wifi router
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  client.setInsecure();
}

void loop() {
  double Irms = SCT013.calcIrms(1480); 
  double I_calib = Irms*1.0548-0.0142;  //use calib factor here
  double power = I_calib * 230;
  
  Serial.print("Current = ");
  Serial.print(Irms);
  Serial.println(" A");
  Serial.print("Power = ");
  Serial.print(power);
  Serial.println(" W");
  Serial.print("...");
  
  sendData(Irms, power, 0); // Call the sendData subroutine
  delay(1000);
}

// Subroutine for sending data to Google Sheets
void sendData(float val1, float val2, float val3) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------

  //----------------------------------------Processing data and sending data
  String string_val1 =  String(val1);
  String string_val2 =  String(val2);
  String string_val3 =  String(val3);
  String url = "/macros/s/" + GAS_ID + "/exec?val1=" + string_val1 + "&val2=" + string_val2 + "&val3="+ string_val3 ;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------
}