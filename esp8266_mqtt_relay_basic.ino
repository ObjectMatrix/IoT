/*
  Basic ESP8266 MQTT for relay controller

  Regis Hsu
  2019-07-10
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>        // Include the mDNS library

#define BUILTIN_LED 2
#define RELAY1_PIN 5
#define RELAY2_PIN 4

#define AP_SSID "your-ssid"
#define AP_PASSWD "password"
#define MQTT_BROKER "xxx.xxx.xxx.xxx"

#define MQTT_PUB "Home/esp32_sub"
#define MQTT_SUB "Home/esp32_pub"

#define RELAY_1_ON  3
#define RELAY_1_OFF 3

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi()
{
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(AP_SSID);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(AP_SSID, AP_PASSWD);

  while ((WiFi.status() != WL_CONNECTED))
  {
    delay(500);
    Serial.print(".");
    digitalWrite(BUILTIN_LED, i % 2);
  }

  //turn off the LED
  digitalWrite(BUILTIN_LED, HIGH);
  randomSeed(micros());
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("esp8266")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

void callback(char* topic, byte * payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  client.publish(MQTT_PUB, (char *)payload);

  // Switch on the LED if an 1 was received as first character
  switch ((char)payload[0] - 48)
  {
    case 0:
      ESP.restart();
      break;
    case 1:
      digitalWrite(RELAY1_PIN, ((char)payload[1] - 48));
      break;
    case 2:
      digitalWrite(RELAY2_PIN, ((char)payload[1] - 48));
      break;
    default:
      Serial.println("Not a command!");
      break;
  }
}

void reconnect() {
  // Loop until we're reconnected
  //while (!client.connected()) {
  Serial.print("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "ESP8266-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  if (client.connect(clientId.c_str())) {
    Serial.println("connected");
    // Once connected, publish an announcement..
    // ... and resubscribe
    client.subscribe(MQTT_SUB);
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in few seconds");
    // Wait 5 seconds before retrying
    delay(5000);
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);    // Initialize the BUILTIN_LED pin as an output
  pinMode(RELAY1_PIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(RELAY2_PIN, OUTPUT);

  Serial.begin(115200);
  delay(500);
  Serial.println();

  digitalWrite(RELAY1_PIN, HIGH);

  setup_wifi();
  client.setServer(MQTT_BROKER, 1883);
  client.setCallback(callback);
  if (!client.connected()) {
    reconnect();
  }
  client.publish(MQTT_PUB, "11");
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  else
    client.loop();

  long now = millis();

  if (now > (RELAY_1_ON * 60 * 1000)) { //ms
    client.publish(MQTT_PUB, "10");
    Serial.printf("Going into deep sleep for %d mins", RELAY_1_OFF);
    client.loop();
    delay(2000);
    ESP.deepSleep(RELAY_1_OFF * 60 * 1000000); //us
  }
}
