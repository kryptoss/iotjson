
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

//#define wifi_ssid "XXXXXXXXXX"
//#define wifi_password "XXXXXXXXX"
#include "wifi_conf.c"

#define topicStatus "status"
#define topicCommands "commands"

#define pinPump 12
#define pinFeeder 14
#define pinLight 16
#define type fishtank

#define msPerSecond 1000
#define eepromAddressTimeBetweenFeeding 0//4 bytes
#define eepromAddressTimeFeeding 4//4 bytes

int statusPump = 0;
int statusFeeder = 0;
int statusLight = 0;
int feederTime=0;
int prevFeederTime=0;

int feedingTime=0;
int prevFeedingTime=0;

int statusFeeding=1;

int loopDelay=250;


int feederCounter=0;//how many loopDelays have been done
int feederCounterLimit=0;//what number feederCounter should reach
int feedingCounter=0;
int feedingCounterLimit=0;

String onString = "on";
String offString = "off";

String id = "fishtank1";


//StaticJsonBuffer<200> jsonBuffer;

DynamicJsonBuffer jsonBuffer;
StaticJsonBuffer<200> jsonBufferAnswer;
JsonObject& rootAnswer = jsonBufferAnswer.createObject();

WiFiClient espClient;
PubSubClient client(espClient);



void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("GW address: ");
  Serial.println(WiFi.gatewayIP());
}


void callback(char* topic_mqtt, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic_mqtt);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();



 JsonObject& root = jsonBuffer.parseObject(payload);
  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  Serial.println("parseObject() success");


  
  //Feeder is connected to a relay normally open. It closes only one second and then it opens agains. So on is closed and off is open. Relay is active at low level, so on is low and off is high;
  if (String((const char *)root["id"]).equalsIgnoreCase(id)) {
    Serial.println("Id ok");
    if (String((const char*) root["feeder"]).equalsIgnoreCase(offString)) {
      Serial.println("feeder HIGH");
      digitalWrite(pinFeeder, HIGH);
    } else if ( String((const char*) root["feeder"]).equalsIgnoreCase(onString)) {
      digitalWrite(pinFeeder, LOW);
      Serial.println("feeder LOW");

    }

  
  //Light is connected to a relay normally closed. So on is closed and off is open. Relay is active at low level, so on is high and off is low;
    if (String((const char*) root["light"]).equalsIgnoreCase(onString)) {
      Serial.println("light HIGH");
      digitalWrite(pinLight, HIGH);

    } else if (String((const char*)  root["light"]).equalsIgnoreCase(offString)) {
      digitalWrite(pinLight, LOW);
      Serial.println("light low");

    }

  
  //pump is connected to a relay normally closed. So on is closed and off is open. Relay is active at low level, so on is high and off is low;
    if (String((const char*) root["pump"]).equalsIgnoreCase(onString)) {
      Serial.println("pump HIGH");
      digitalWrite(pinPump, HIGH);
    } else if (String((const char*)root["pump"]).equalsIgnoreCase(offString)) {
      digitalWrite(pinPump, LOW);
      Serial.println("pump low");
    }

  //pump is connected to a relay normally closed. So on is closed and off is open. Relay is active at low level, so on is high and off is low;
   if (String((const char*) root["feederTime"])!="0") {
    Serial.println("Time: "+ String((const char*) root["feederTime"]));
    feederTime=String((const char*) root["feederTime"]).toInt();
    EEPROM.get(eepromAddressTimeBetweenFeeding, prevFeederTime);
    if(feederTime!=prevFeederTime){
        Serial.println("Write eeprom nee feederTime");
        EEPROM.put(eepromAddressTimeBetweenFeeding,feederTime);   
        EEPROM.commit();
    } 
    Serial.println("EEPROM Time: "+ String(EEPROM.get(eepromAddressTimeBetweenFeeding, feederTime)));
   }
    

    if (String((const char*) root["loopDelay"])!="0") {//this only try if it exists, otherway it reboots
        Serial.println("loopDelay: "+ String((const char*) root["loopDelay"]));
        loopDelay=String((const char*) root["loopDelay"]).toInt();
    
    }

  }

 sendStatus();

  
}

void sendStatus(){
  
  
   //JsonObject& rootAnswer = jsonBufferAnswer.createObject();
//JsonObject& rootAnswer = jsonBufferAnswer.createObject();


  rootAnswer["id"] = id;
  
  rootAnswer["pump"] = digitalRead(pinPump) ? "off" : "on";
  rootAnswer["feeder"] = digitalRead(pinFeeder) ? "off" : "on";
  rootAnswer["light"] = digitalRead(pinLight) ? "off" : "on";
  rootAnswer["feederTime"] = String(EEPROM.get(eepromAddressTimeBetweenFeeding, feederTime));
  rootAnswer["feedingTime"] = String(EEPROM.get(eepromAddressTimeFeeding, feederTime));
  rootAnswer["loopDelay"] = String(loopDelay);
  
  Serial.println("Status");
  Serial.println("pump: " + String((const char*) rootAnswer["pump"]));
  Serial.println("light: " + String((const char*)  rootAnswer["light"]));
  Serial.println("feeder: " + String((const char*)  rootAnswer["feeder"]));
  Serial.println("feederTime: " + String((const char*)  rootAnswer["feederTime"]));
    Serial.println("feedingTime: " + String((const char*)  rootAnswer["feedingTime"]));
  Serial.println("loopDelay: " + String((const char*)  rootAnswer["loopDelay"]));
 char answer[200];
 rootAnswer.printTo(answer);
 client.publish(topicStatus, answer);
  
  
  }


void setup() {

  Serial.begin(115200);
  Serial.print("Starting device...");
  setup_wifi();
  client.setServer(WiFi.gatewayIP(), 1883);//router should redirect petitions to actual server
  Serial.println("MQTT_server");


  EEPROM.begin(512);
  client.setCallback(callback);
  Serial.println("callback");


  feederCounterLimit=feederTime*(msPerSecond/loopDelay);
  

  //char* topicMqtt="mqtt_topic";
  //char* payloadMqtt="payload";

  //callback((char*)topicMqtt, (byte*)payloadMqtt, 7);
  pinMode(pinPump, OUTPUT);
  pinMode(pinFeeder, OUTPUT);
  pinMode(pinLight, OUTPUT);
  EEPROM.get(eepromAddressTimeBetweenFeeding, feederTime);
  EEPROM.get(eepromAddressTimeFeeding, feedingTime);
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    if (client.connect("ESP8266Client")) {
      // if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(topicCommands);
      Serial.println("subscribed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
   sendStatus();
  }
  
}



void loop() {
  if (!client.connected()) {
    reconnect();
    Serial.println("Trying reconnect MQTT");
 
  } 
// else {
//    Serial.println("MQTT connected");
//  }
  
  client.loop();//pubsub client loop
  //Serial.println("MQTT loop executed");
  
  //Serial.println("no envio nada");
  delay(loopDelay);

  if(feederCounter==feederCounterLimit){
    digitalWrite(pinLight, LOW);//light on
    digitalWrite(pinPump, LOW);//pump off
    digitalWrite(pinFeeder, LOW);//press feeder button
    delay(100);
    digitalWrite(pinFeeder, HIGH);//release feeder button
    statusFeeding=0;
   }else{
      feederCounter++;
    }

  if(statusFeeding){
    feedingCounter++;
    if(feedingCounter==feedingCounterLimit){
          statusFeeding=1;
          digitalWrite(pinPump, HIGH);//pump on
          feederCounter=0;
          feedingCounter=0;
      }
    }
    
}

