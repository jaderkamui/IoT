#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

#include "DHT.h"
#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

#define AWS_IOT_PUBLISH_TOPIC   "DHT_Sensor/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "DHT_Sensor/sub"

float h ;
float t;


DHT dht(DHTPIN, DHTTYPE);

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Conectando al Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi conectado: ");
  Serial.println(WIFI_SSID);
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Create a message handler
  client.setCallback(messageHandler);

  Serial.println("Conectando por MQTT a AWS IOT");

  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(200);
  }

  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("Certificados AWS IoT Correctos");
  Serial.println("******************************************");
  Serial.println("Conectado correctamente!");
  digitalWrite(22, HIGH);
}

void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["humedad"] = h;
  doc["temperatura"] = t;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);

}



void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}

void setup()
{
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  Serial.begin(115200);
  connectAWS();
  dht.begin();
}

void loop()
{
  h = dht.readHumidity();
  t = dht.readTemperature();


  if (isnan(h) || isnan(t) )  // Check if any reads failed and exit early (to try again).
  {
    Serial.println(F("Falla en detectar DHT sensor, Reinicie equipo!"));
    return;
  }

  digitalWrite(23, LOW);
  Serial.print(F("Humedad Ambiental: "));
  Serial.print(h);
  Serial.print(F("%  Temperatura Ambiental: "));
  Serial.print(t);
  Serial.println(F("°C "));
  Serial.println(F("Envio MQTT con Exito a AWS IoT Core ..."));
  publishMessage();
  client.loop();
  delay(2000);
  digitalWrite(23, HIGH);
}
