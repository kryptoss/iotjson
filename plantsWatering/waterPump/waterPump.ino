
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

//#define wifi_ssid "XXXXXXXXXX"
//#define wifi_password "XXXXXXXXX"
//#define MQTT_server "XXXXXXXXXX"
#include "wifi_conf.c"

#define topicStatus "status"
#define topicCommands "commands"

#define pinPump 12
#define pinValve 14
#define pinLevelSensor 5
#define type waterpump

#define msPerSecond 1000
int mainLoopDelay=24*59*60;//A day minus a minute, in seconds
int mainActionDelay=60;//in seconds
int aliveDelay=3600;//in seconds

int onByUser=false;
int statusPump = 0;
int performingAction=false;

int serialDebug=false;

int loopDelay=500;//main loop delay in ms

int loopsCounter=0;//how many loop Delays have been done
int loopsCounterLimit=0;//what number loopsCounter should reach

int loopsActionCounter=0;//how many loops pump is on
int loopsActionCounterLimit=0;//how many loops pump should be on
int loopsAlivesCounter=0;
int loopsAlivesCounterLimit=0;//how many loops between alives

String onString = "on";
String offString = "off";

String id = "waterPump1";


//StaticJsonBuffer<200> jsonBuffer;

DynamicJsonBuffer jsonBuffer;
StaticJsonBuffer<300> jsonBufferAnswer;
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

  if(serialDebug){
      Serial.print("Message arrived [");
      Serial.print(topic_mqtt);
      Serial.print("] ");
      for (int i = 0; i < length; i++) {
          Serial.print((char)payload[i]);
      }
      Serial.println();
  }


 JsonObject& root = jsonBuffer.parseObject(payload);
  // Test if parsing succeeds.
  if (!root.success()) {
    if(serialDebug) Serial.println("parseObject() failed");
    return;
  }
  if(serialDebug) Serial.println("parseObject() success");


  
  //Feeder is connected to a relay normally open. It closes only one second and then it opens agains. So on is closed and off is open. Relay is active at low level, so on is low and off is high;
  if (String((const char *)root["id"]).equalsIgnoreCase(id)) {
    if(serialDebug) Serial.println("Id ok");

 if (root.containsKey("mainLoopDelay")) {
        mainLoopDelay=String((const char*) root["mainLoopDelay"]).toInt();
        setLoopValues();
    } 
 
 if (root.containsKey("mainActionDelay")) {
        mainActionDelay=String((const char*) root["mainActionDelay"]).toInt();
        setLoopValues();
    }  

  if (root.containsKey("aliveDelay")) {
        aliveDelay=String((const char*) root["aliveDelay"]).toInt();
        setLoopValues();
    }

    if (String((const char*) root["reset"]).equalsIgnoreCase("true")) {
        loopsCounter=0;
        loopsActionCounter=0;
        loopsAlivesCounter=0;
    }  


    if (String((const char*) root["serialDebug"]).equalsIgnoreCase("true")) {
        serialDebug=true;

    } else if (String((const char*)root["serialDebug"]).equalsIgnoreCase("false")) {
      serialDebug=false;
    }
    
  
  //pump is connected to a relay normally closed. So on is closed and off is open. Relay is active at low level, so on is high and off is low;
    if (String((const char*) root["pump"]).equalsIgnoreCase(onString)) {
        startMainAction();
        onByUser=true;

    } else if (String((const char*)root["pump"]).equalsIgnoreCase(offString)) {
      stopMainAction();
    }





  }

 sendStatus();

  
}

void sendStatus(){
  
  
   //JsonObject& rootAnswer = jsonBufferAnswer.createObject();
//JsonObject& rootAnswer = jsonBufferAnswer.createObject();


  rootAnswer["id"] = id;
  
  rootAnswer["pump"] = digitalRead(pinPump) ? "off" : "on";
  rootAnswer["mainActionDelay"] = String(mainActionDelay);
  rootAnswer["mainLoopDelay"] = String(mainLoopDelay);
  rootAnswer["aliveDelay"] = String(aliveDelay);
  rootAnswer["tankEmpty"] = digitalRead(pinLevelSensor) ? "true" : "false";
  rootAnswer["serialDebug"] = serialDebug ? "true" : "false";
  rootAnswer["currentMainCounter"] = String(loopsCounter);
  rootAnswer["currentActionCounter"] = String(loopsActionCounter);
  rootAnswer["currentAlivesCounter"] = String(loopsAlivesCounter);
  rootAnswer["reset"] = String("false");

  if(serialDebug){
  Serial.println("Status");
  Serial.println("pump: " + String((const char*) rootAnswer["pump"]));
  Serial.println("mainActionDelay: " + String((const char*)  rootAnswer["mainActionDelay"]));
  Serial.println("mainLoopDelay: " + String((const char*)  rootAnswer["mainLoopDelay"]));
  Serial.println("aliveDelay: " + String((const char*)  rootAnswer["aliveDelay"]));
  Serial.println("tankEmpty: " + String((const char*)  rootAnswer["tankEmpty"]));
  Serial.println("serialDebug: " + String((const char*)  rootAnswer["serialDebug"]));
  Serial.println("currentMainCounter: " + String((const char*)  rootAnswer["currentMainCounter"]));
  Serial.println("currentActionCounter: " + String((const char*)  rootAnswer["currentActionCounter"]));
  Serial.println("currentAlivesCounter: " + String((const char*)  rootAnswer["currentAlivesCounter"]));
  Serial.println("reset: " + String((const char*)  rootAnswer["reset"]));
  
  }
 char answer[200];
 rootAnswer.printTo(answer);
  
 client.publish(topicStatus, answer);
  
  
  }


void setup() {

  Serial.begin(115200);
  Serial.print("Starting device...");
  setup_wifi();
  //client.setServer(WiFi.gatewayIP(), 1883);//router should redirect petitions to actual server
  client.setServer(MQTT_server, 1883);//router should redirect petitions to actual server
  Serial.println("MQTT_server set");


  client.setCallback(callback);
  Serial.println("callback set");


  setLoopValues();

  //char* topicMqtt="mqtt_topic";
  //char* payloadMqtt="payload";

  //callback((char*)topicMqtt, (byte*)payloadMqtt, 7);
  pinMode(pinPump, OUTPUT);
  pinMode(pinValve, OUTPUT);
  pinMode(pinLevelSensor,INPUT);
  digitalWrite(pinPump,HIGH);//HIGH is off
  digitalWrite(pinValve,HIGH);
  sendStatus();
}


void setLoopValues(){

  loopsCounterLimit=mainLoopDelay*(msPerSecond/loopDelay);//mainLoop in seconds, loopDelay in miliseconds
  loopsActionCounterLimit=mainActionDelay*(msPerSecond/loopDelay);//action loop in seconds, loopDelay in miliseconds
  loopsAlivesCounterLimit=aliveDelay*(msPerSecond/loopDelay);//action loop in seconds, loopDelay in miliseconds
  if(serialDebug){
      Serial.println("counterLimit: "+String(loopsCounterLimit));
      Serial.println("actionCounterLimit: "+String(loopsActionCounterLimit));
  }
  }


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    if(serialDebug) Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    if (client.connect("ESP8266Client")) {
      // if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      if(serialDebug) Serial.println("connected");
      client.subscribe(topicCommands);
      if(serialDebug)Serial.println("subscribed");
    } else {
      if(serialDebug){
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      }
      // Wait 5 seconds before retrying
      delay(5000);
    }
   sendStatus();
  }
  
}


void startMainAction(){
    if(serialDebug)Serial.println("pump on");
    digitalWrite(pinPump, LOW);//pump on
    digitalWrite(pinValve, LOW);//valve on
    performingAction=true;
    loopsActionCounter=0;
    sendStatus();

}

void stopMainAction(){
    if(serialDebug)Serial.println("pump off");
    digitalWrite(pinPump, HIGH);//pump off
    digitalWrite(pinValve, HIGH);//pump off
    performingAction=false;
    loopsActionCounter=0;
    sendStatus();
    
}



void loop() {
  if (!client.connected()) {
    reconnect();
    if(serialDebug) Serial.println("Trying reconnect MQTT");
 
  } 
// else {
//    Serial.println("MQTT connected");
//  }
  
  client.loop();//pubsub client loop
  //Serial.println("MQTT loop executed");
  
  //Serial.println("no envio nada");


  //feederCounter: time to next feeding Time
  //feedingCounter: time with stopped water pump while fish can eat food

  if(loopsCounter>=loopsCounterLimit){
    if(serialDebug) Serial.println("Enough loops");
    if(!performingAction){
      if(serialDebug) Serial.println("start action loop");
      startMainAction();
  
    }
  }else{
    if(serialDebug)Serial.println("Loops:"+String(loopsCounter));
    loopsCounter++;
  }


  if(performingAction){
    loopsActionCounter++;
        if(serialDebug) Serial.println("LoopsAction:"+String(loopsActionCounter));
    if(loopsActionCounter>=loopsActionCounterLimit){
          if(serialDebug) Serial.println("Enough action loops");
          stopMainAction();
          if(!onByUser){
              loopsCounter=0;
          }else{
              onByUser=false;
          }
    }
  }


  if(loopsAlivesCounter>=loopsAlivesCounterLimit){
    sendStatus();
    loopsAlivesCounter=0;
    }else{
      loopsAlivesCounter++;
      }
    
  delay(loopDelay);
}

