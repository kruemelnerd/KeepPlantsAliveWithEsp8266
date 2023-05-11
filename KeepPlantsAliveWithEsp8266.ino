#include <ESP8266WiFi.h>  // Include the Wi-Fi library
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "secrets.h"



//Your Domain name with URL path or IP address with path
const char* serverName = "http://192.168.178.33:8080/api/saveSoilMoisture";


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

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");
    // Data to send with HTTP POST
    String httpRequestData = String("{ \"device\": \"raging rackoon\", \"room\": \"bathroom\", \"numberInRoom\": 4,\"soilMoisture\": ") + output_value + ", \"dateTime\": \"2023-05-10T16:07:09\"}";
    Serial.print("serverName: ");
    Serial.println(serverName);

    Serial.print("Request: ");
    Serial.println(httpRequestData);
    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);
    Serial.print("ResponseCode: ");
    Serial.println(httpResponseCode);
    // Free resources
    http.end();
  } else {
    Serial.println("WiFi is Disconnected!");
  }
  delay(2000);
  //ESP.deepSleep(5e6);
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
