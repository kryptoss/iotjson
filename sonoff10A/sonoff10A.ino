
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>




//#define wifi_ssid "XXXXXXXXXX"
//#define wifi_password "XXXXXXXXX"



//Relay GPIO12 - pin 10
#define relayPin 10 


String onString = "on";
String offString = "off";
#define answerBufferSize 200
char answerBuffer[answerBufferSize];

#include "wifi_conf.c"




#define msPerSecond 1000

unsigned int sleepDelay=20E6;



//StaticJsonBuffer<200> jsonBuffer;

DynamicJsonBuffer jsonBuffer;
StaticJsonBuffer<200> jsonBufferAnswer;
//JsonObject& rootAnswer = jsonBufferAnswer.createObject();

WiFiClient espClient;
PubSubClient client(espClient);




#define topicReceivedMeter "switches"
#define topicSendMeter "controlswitches"


String id = "switch-0";
unsigned int switchState=true;
unsigned int prevSwitchState=true;

void sendMQTTStatus(){
  JsonObject& rootAnswer = jsonBufferAnswer.createObject();
  rootAnswer["id"] = id;
  rootAnswer["state"] = String(switchState);
  rootAnswer["sleepDelay"] = String(sleepDelay);
  
  Serial.println("Send meter");
  Serial.println("state: " + String((const char*) rootAnswer["state"]));
  Serial.println("sleepDelay: " + String((const char*)  rootAnswer["sleepDelay"]));
 char answer[200];
 rootAnswer.printTo(answer);
 client.publish(topicSendMeter, answer);
 cleanAnswerBuffer();
 jsonBufferAnswer.clear();
}






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






void callbackMQTT(char* topic_mqtt, byte* payload, unsigned int length) {

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



  
  if (String((const char *)topic_mqtt).equalsIgnoreCase(topicSendMeter)) {
    if (String((const char *)root["id"]).equalsIgnoreCase(id)) {
      Serial.println("Id ok");
      if (String((const char*) root["switchState"])!="0") {//this only try if it exists, otherway it reboots
          if (String((const char*) root["switchState"]).equalsIgnoreCase(onString)) {
              prevSwitchState=switchState;
              switchState=true;
              Serial.println("state on");
          }else if (String((const char*) root["switchState"]).equalsIgnoreCase(offString)) {
              prevSwitchState=switchState;
              switchState=false;
              Serial.println("state off");
          } 
      if (String((const char*) root["sleepDelay"])!="0") {//this only try if it exists, otherway it reboots
          sleepDelay = String((const char*) root["sleepDelay"]).toInt() ;
              Serial.print("sleep delay: ");
              Serial.println(sleepDelay);
          }
     
     }else{
      Serial.println("no state command");
     }
          
    }else{
      Serial.println("Id wrong");
    }
  }else{ 
      Serial.println("Wrong topic");
  }
  
}




void setup() {

  Serial.begin(115200);
  Serial.print("Starting device...");
  setup_wifi();
  client.setServer(WiFi.gatewayIP(), 1883);//router should redirect petitions to actual server
  //client.setServer(mqtt_server, 1883);//router should redirect petitions to actual server
  Serial.println("MQTT_server");


  client.setCallback(callbackMQTT);
  Serial.println("callbackMQTT");


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
      client.subscribe(topicReceivedMeter);
      Serial.println("subscribed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }

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


  if(switchState!=prevSwitchState){
      if(switchState){
          digitalWrite(relayPin, HIGH);
          sendMQTTStatus();
      }else if(!switchState){

          digitalWrite(relayPin, LOW);
          sendMQTTStatus();
      }
  }




 // Serial.println("sleep");
  
  //connect GPIO16 pin to RST pin
 // ESP.deepSleep(sleepDelay);

 delay(5000);//delay 5 segs
  
    
}

void cleanAnswerBuffer(){
   for(int itr=0; itr<answerBufferSize; itr++){
    answerBuffer[itr]=0;
    
    }
}

