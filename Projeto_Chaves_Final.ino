#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "DHT.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DHTPIN 4 
#define DHTTYPE DHT11

const char* ssid = "Flamengo";
const char* password = "997565941";

const char* mqtt_server = "henriqueifsp.duckdns.org";

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

const int bomba = 26;
const int bomba2 = 32;
const int umidade = 34;

long lastMsg = 0;
char msg[50];
int value = 0;
float h, hu, t;


//*****************************************************
void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);
  //xTaskCreate(leumidade_task, "le umidade", 5000, NULL, 3, NULL);
  xTaskCreate(letemperatura_task, "le temperatura", 5000, NULL, 4, NULL);
  pinMode (bomba, OUTPUT);
  pinMode (bomba2, OUTPUT);
}

//*****************************************************
/*void leumidade_task(void *p)
{
    while (1)
    {
        delay(1);
        h = analogRead(umidade);
        hu = map(h,0,4095,0,100);
    }
}*/
//*****************************************************
void letemperatura_task(void *p)
{
    while (1)
    {
        delay(1);
        t = dht.readTemperature(); 
    }
}
//*****************************************************        
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

//*****************************************************
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


  if (String(topic) == "esp32/output1"){ 
    Serial.print("Changing output to ");
     if(messageTemp == "liga")
     {
      digitalWrite(bomba2, HIGH);
      Serial.printf("bomba 2 ligada");
      client.publish("bomba2", "ligado");
     }
     else if (messageTemp == "desliga")
     {
      digitalWrite(bomba2, LOW);
      Serial.printf("bomba 2 desligada"); 
      client.publish("bomba2", "desligado");
     }
  }
}
 

//*****************************************************

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//*****************************************************
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;}
    
long now = millis();
   if (now - lastMsg > 5000) 
   {
     lastMsg = now;

     h = analogRead(umidade);
     hu = map(h,0,4095,100,0);
     
     if (hu <= 30)
     {
      digitalWrite (bomba, HIGH);
      client.publish("bomba1", "ligado");
     }
     else 
     {
      digitalWrite (bomba, LOW);
      client.publish("bomba1", "desligado");
     }
     
     // Convert the value to a char array
     char tempString2[8];
     dtostrf(t, 1, 2, tempString2);
     Serial.print("t: ");
     Serial.println(tempString2);
     client.publish("Temperatura", tempString2);
     
     char tempString[8];
     dtostrf(hu, 1, 2, tempString);
     Serial.print("hu: ");
     Serial.println(tempString);
     client.publish("umidade", tempString);
   }

}
