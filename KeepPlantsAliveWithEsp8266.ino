#include <ESP8266WiFi.h>  // Include the Wi-Fi library
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets.h"




#define AWS_IOT_PUBLISH_TOPIC   "esp8266/pub"


int sensor_pin = A0;
int output_value;
#define LED 2  //Define blinking LED pin

void setup() {
  Serial.begin(9600);
  connectToWifi();
  pinMode(LED, OUTPUT);
  Serial.println("Reading from Sensors ...");
}

// the loop function runs over and over again forever
void loop() {
  output_value = analogRead(sensor_pin);
  // output_value = map(output1, sensor1Wet, sensor1Dry, 100, 0);
  output_value = map(output_value, 280, 680, 100, 0);
  Serial.print("Moisture: ");
  Serial.print(output_value);
  Serial.println("%");
  if (output_value >= 30) {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }
  ESP.deepSleep(5e6);
}

void connectToWifi() {
  WiFi.begin(ssid, password);  // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {  // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i);
    Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());  // Send the IP address of the ESP8266 to the computer
}