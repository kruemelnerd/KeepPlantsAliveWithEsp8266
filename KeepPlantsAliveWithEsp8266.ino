#include <ESP8266WiFi.h>  // Include the Wi-Fi library
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets.h"




time_t now;
time_t nowish = 1510592825;
unsigned long lastMillis = 0;
unsigned long previousMillis = 0;

#define AWS_IOT_PUBLISH_TOPIC "esp8266_water/pub"

WiFiClientSecure net;

BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);

PubSubClient client(net);


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


  now = time(nullptr);

  if (!client.connected()) {
    connectAWS();
  } else {
    client.loop();
    if (millis() - lastMillis > 5000) {
      lastMillis = millis();
      publishMessage(output_value);
    }
  }
  delay(5000);
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


void NTPConnect(void) {
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void connectAWS(void) {
  connectToWifi();

  NTPConnect();

  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);

  client.setServer(MQTT_HOST, 8883);
  client.setCallback(messageReceived);

  Serial.println("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(1000);
  }

  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  // Subscribe to a topic
  //client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void messageReceived(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void publishMessage(int output_value)
{
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["output_value"] = output_value;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

