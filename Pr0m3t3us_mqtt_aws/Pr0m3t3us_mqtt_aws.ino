#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
 
#include "DHT.h"
#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
 
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

int AN_In1 = 36;// GPIO 36 is Now AN Input 1
int AN_In2 = 39;// GPIO 39 is Now AN Input 2
int AN_In3 = 34;// GPIO 34 is Now AN Input 3
int AN_In4 = 35;// GPIO 35 is Now AN Input 4
int AN_In5 = 32;// GPIO 32 is Now AN Input 5

 
float h ;
float t;
int s1;
int s2;
int s3;
int s4;
int s5;
 
DHT dht(DHTPIN, DHTTYPE);
 
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
 
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Prometeo Conectando al Wi-Fi");
 
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
 
  Serial.println("Prometeo Conectado a AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
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
  Serial.println("Conectado correctamente por MQTT!");
  Serial.println("******************************************");
  Serial.println(".--.--. Prometeo Listo .--.--.");
  Serial.println("******************************************");
  digitalWrite(22, HIGH);
}
 
void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["humedad"] = h;
  doc["temperatura"] = t;
  doc["Sensor1"] = s1;
  doc["Sensor2"] = s2;
  doc["Sensor3"] = s3;
  doc["Sensor4"] = s4;
  doc["Sensor5"] = s5;
  
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
  s1 = analogRead(AN_In1);
  s2 = analogRead(AN_In2);
  s3 = analogRead(AN_In3);
  s4 = analogRead(AN_In4);
  s5 = analogRead(AN_In5);
 
 
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
  Serial.print(F("Sensor Aire 1 : "));
  Serial.print(s1);
  Serial.println(F(" "));
  Serial.print(F("Sensor Aire 2 : "));
  Serial.print(s2);
  Serial.println(F(" "));
  Serial.print(F("Sensor Aire 3 : "));
  Serial.print(s3);
  Serial.println(F(" "));
  Serial.print(F("Sensor Aire 4 : "));
  Serial.print(s4);
  Serial.println(F(" "));
  Serial.print(F("Sensor Aire 5 : "));
  Serial.print(s5);
  Serial.println(F(" "));
  Serial.println(F("... Envio MQTT con Exito a AWS IoT Core ...")); 
  publishMessage();
  client.loop();
  delay(2000);
  digitalWrite(23, HIGH);
}
