#include <ESP8266WiFi.h>  // Include the Wi-Fi library
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "time.h"
#include "secrets.h"

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;      //Replace with your GMT offset (seconds)
const int daylightOffset_sec = 3600;  //Replace with your daylight offset (seconds)

/* Globals */
time_t now;  // this is the epoch
tm tm;       // the structure tm holds time information in a more convenient way

// 60 min x 60 sec * 1000 millisec * 1000 Microsec = 3600
const int timeDeepSleepMinutes = 60;
const int timeDeepSleepMicrosec = timeDeepSleepMinutes * 60 * 1000 * 1000;


//Your Domain name with URL path or IP address with path
const char* serverName = "http://192.168.178.13:8080/api/saveSoilMoisture";

//Zeitverschiebung UTC <-> MEZ (Winterzeit) = 3600 Sekunden (1 Stunde)
//Zeitverschiebung UTC <-> MEZ (Sommerzeit) = 7200 Sekunden (2 Stunden)
//const long utcOffsetInSeconds = 3600 * 2;

int sensor_pin = A0;
int output_value;
#define LED D4  //Define blinking LED pin

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  connectToWifi();
  setupTime();
  Serial.println("Reading from Sensors ...");
}

void setupTime() {
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  getLocalDateTime();
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, LOW);
  output_value = analogRead(sensor_pin);
  // output_value = map(output1, sensor1Wet, sensor1Dry, 100, 0);
  output_value = map(output_value, 280, 680, 100, 0);
  Serial.print("Moisture: ");
  Serial.print(output_value);
  Serial.println("%");
  if (output_value >= 30) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  if (WiFi.status() == WL_CONNECTED) {

    getLocalDateTime();

    WiFiClient client;
    HTTPClient http;
    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");
    // Data to send with HTTP POST
    String httpRequestData = String("{ \"device\": \"raging rackoon\", \"room\": \"bathroom\", \"numberInRoom\": 4,\"soilMoisture\": ") + output_value + ", \"dateTime\": \"" + getLocalDateTime() + "\"}";
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
  //delay(2000);
  Serial.println("Sleeping for " + String(timeDeepSleepMicrosec) + " microsec.");
  ESP.deepSleep(timeDeepSleepMicrosec);
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

String getLocalDateTime() {

  time(&now);              // read the current time
  localtime_r(&now, &tm);  // update the structure tm with the current time

  String year = String(tm.tm_year + 1900);
  while (year == "1970") {
    Serial.println("Year is 1970: Check Time again.");
    delay(1000);
    time(&now);              // read the current time
    localtime_r(&now, &tm);  // update the structure tm with the current time
    year = String(tm.tm_year + 1900);
  }

  //String month = String(tm.tm_mon + 1);
  //if(tm.tm_mon + 1 < 10){
  //    month = "0" + month;
  //  }
  String month = formatSmallerNumbers(tm.tm_mon + 1);
  String day = formatSmallerNumbers(tm.tm_mday);
  String hour = formatSmallerNumbers(tm.tm_hour);
  String minute = formatSmallerNumbers(tm.tm_min);
  String sec = formatSmallerNumbers(tm.tm_sec);
  String localDateTime = String(tm.tm_year + 1900) + "-" + month + "-" + day + "T" + hour + ":" + minute + ":" + sec;
  Serial.print("LocalDateTime: ");
  Serial.println(localDateTime);
  return localDateTime;
}

String formatSmallerNumbers(int number) {
  if (number < 10) {
    return String("0") + String(number);
  }
  return String(number);
}
