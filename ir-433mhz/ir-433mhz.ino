
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>




//#define wifi_ssid "XXXXXXXXXX"
//#define wifi_password "XXXXXXXXX"
#include "wifi_conf.c"

#define mqtt_server "192.168.1.142"


#define pinLedIR 12
#define pinPhotoIR 14
#define pinSender433 4
#define pinReceiver433 5

#define CAPABLE433 
#define CAPABLEIR  

#define msPerSecond 1000


int loopDelay=250;



String onString = "on";
String offString = "off";
#define answerBufferSize 200
char answerBuffer[answerBufferSize];




//StaticJsonBuffer<200> jsonBuffer;

DynamicJsonBuffer jsonBuffer;
StaticJsonBuffer<200> jsonBufferAnswer;
//JsonObject& rootAnswer = jsonBufferAnswer.createObject();

WiFiClient espClient;
PubSubClient client(espClient);








#ifdef CAPABLEIR

#define topicReceivedIR "receivedIR"
#define topicSendIR "sendIR"

#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

String idIR = "ir-1";
unsigned long codeIR=0;
unsigned int bitsIR=0;


IRrecv irrecv(pinPhotoIR);
IRsend irsend(pinLedIR);  // an IR led is connected to GPIO pin 4 (D2)
decode_results results;  // Somewhere to store the results
irparams_t save;         // A place to copy the interrupt state while decoding.

void sendIR(int code, int bits){
  
    irsend.sendNEC(code, bits);
}


// Display encoding type
//
void encoding(decode_results *results) {
  switch (results->decode_type) {
    default:
    case UNKNOWN:      Serial.print("UNKNOWN");       break;
    case NEC:          Serial.print("NEC");           break;
    case SONY:         Serial.print("SONY");          break;
    case RC5:          Serial.print("RC5");           break;
    case RC6:          Serial.print("RC6");           break;
    case DISH:         Serial.print("DISH");          break;
    case SHARP:        Serial.print("SHARP");         break;
    case JVC:          Serial.print("JVC");           break;
    case SANYO:        Serial.print("SANYO");         break;
    case SANYO_LC7461: Serial.print("SANYO_LC7461");  break;
    case MITSUBISHI:   Serial.print("MITSUBISHI");    break;
    case SAMSUNG:      Serial.print("SAMSUNG");       break;
    case LG:           Serial.print("LG");            break;
    case WHYNTER:      Serial.print("WHYNTER");       break;
    case AIWA_RC_T501: Serial.print("AIWA_RC_T501");  break;
    case PANASONIC:    Serial.print("PANASONIC");     break;
    case DENON:        Serial.print("DENON");         break;
    case COOLIX:       Serial.print("COOLIX");        break;
  }
  if (results->repeat) Serial.print(" (Repeat)");
}

// Dump out the decode_results structure.
//
void dumpCode(decode_results *results) {
  // Start declaration
  Serial.println("dumpcode received");
  Serial.print("uint16_t  ");              // variable type
  Serial.print("rawData[");                // array name
  Serial.print(results->rawlen - 1, DEC);  // array size
  Serial.print("] = {");                   // Start declaration

  // Dump data
  for (uint16_t i = 1; i < results->rawlen; i++) {
    Serial.print(results->rawbuf[i] * USECPERTICK, DEC);
    if (i < results->rawlen - 1)
      Serial.print(",");  // ',' not needed on last one
    if (!(i & 1)) Serial.print(" ");
  }

  // End declaration
  Serial.print("};");  //

  // Comment
  Serial.print("  // ");
  encoding(results);
  Serial.print(" ");
  serialPrintUint64(results->value, 16);

  // Newline
  Serial.println("");

  // Now dump "known" codes
  if (results->decode_type != UNKNOWN) {
    // Some protocols have an address &/or command.
    // NOTE: It will ignore the atypical case when a message has been decoded
    // but the address & the command are both 0.
    if (results->address > 0 || results->command > 0) {
      Serial.print("uint32_t  address = 0x");
      Serial.print(results->address, HEX);
      Serial.println(";");
      Serial.print("uint32_t  command = 0x");
      Serial.print(results->command, HEX);
      Serial.println(";");
    }

    // All protocols have data
    Serial.print("uint64_t  data = 0x");
    serialPrintUint64(results->value, 16);
    Serial.println(";");
  }
}


void sendMQTTIR(decode_results *results){
  JsonObject& rootAnswer = jsonBufferAnswer.createObject();
  rootAnswer["id"] = idIR;
  rootAnswer["bits"] = String((int)results->bits);
  //rootAnswer["code"] = bin2hex(results->value); 
  rootAnswer["code"] = String((unsigned int)results->value,16);
  
  Serial.println("Received IR");
  Serial.println("code: " + String((const char*) rootAnswer["code"]));
  Serial.println("bits: " + String((const char*)  rootAnswer["bits"]));
 char answer[200];
 rootAnswer.printTo(answer);
 client.publish(topicReceivedIR, answer);
 cleanAnswerBuffer();
 jsonBufferAnswer.clear();
}


#endif



#ifdef CAPABLE433

#define topicReceived433 "received433"
#define topicSend433 "send433"

#include <RCSwitch.h>

String id433 = "433-1";
unsigned int code433=0;
unsigned int bits433=0;
RCSwitch mySwitch = RCSwitch();

void send433(int code, int bits){
  
    mySwitch.send(code,bits);
}

void sendMQTT433(unsigned int code, unsigned int bits){
  JsonObject& rootAnswer = jsonBufferAnswer.createObject();
  rootAnswer["id"] = id433;
  
  rootAnswer["code"] = String(code);
  rootAnswer["bits"] = String(bits);
  
  Serial.println("Send 433");
  Serial.print("rawcode: ");
  Serial.println(code);
  Serial.print("String code: ");
  Serial.println(String(code));
  Serial.print("bits: ");
  Serial.println(bits);
  Serial.println("code: " + String((const char*) rootAnswer["code"]));
  Serial.println("bits: " + String((const char*)  rootAnswer["bits"]));
 char answer[200];
 rootAnswer.printTo(answer);
 client.publish(topicReceived433, answer);
 cleanAnswerBuffer();
 jsonBufferAnswer.clear();
}

#endif

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



#ifdef CAPABLEIR
  
  if (String((const char *)topic_mqtt).equalsIgnoreCase(topicSendIR)) {
    if (String((const char *)root["id"]).equalsIgnoreCase(idIR)) {
      Serial.println("Id ir ok");
      if (String((const char*) root["code"])!="0") {//this only try if it exists, otherway it reboots
          Serial.println(String((const char*) root["code"]));
          Serial.println(String((const char*) root["bits"]));
          String hexcodeIR=String((const char*) root["code"]);
          
          Serial.println(hexcodeIR);
          Serial.println("---");
          
          
          bitsIR=String((const char*) root["bits"]).toInt();
          Serial.print("code dec: ");
          codeIR=strtoul(hexcodeIR.c_str(),NULL,16);
          Serial.println(codeIR,DEC);
          Serial.println(codeIR,HEX);
          Serial.print("bits: ");
          Serial.println(bitsIR);
          Serial.println("end sending");
          sendIR(codeIR,bitsIR);
      }
    }
  }
#endif

#ifdef CAPABLE433

  if (String((const char *)topic_mqtt).equalsIgnoreCase(topicSend433)) {
      if (String((const char *)root["id"]).equalsIgnoreCase(id433)) {
        Serial.println("Id 433 ok");
        if (String((const char*) root["code"])!="0") {//this only try if it exists, otherway it reboots
            code433=String((const char*) root["code"]).toInt();
            bits433=String((const char*) root["bits"]).toInt();
            Serial.print("code: ");
            Serial.print(code433);
            Serial.print(", bits: ");
            Serial.print(bits433);
            send433(code433,bits433);
        }
     
      }
  }

#endif
  
}




void setup() {

  Serial.begin(115200);
  Serial.print("Starting device...");
  setup_wifi();
  //client.setServer(WiFi.gatewayIP(), 1883);//router should redirect petitions to actual server
  client.setServer(mqtt_server, 1883);//router should redirect petitions to actual server
  Serial.println("MQTT_server");


  client.setCallback(callbackMQTT);
  Serial.println("callbackMQTT");


#ifdef CAPABLEIR
  pinMode(pinLedIR, OUTPUT);
  pinMode(pinPhotoIR, INPUT);
  irrecv.enableIRIn();  // Start the receiver
  irsend.begin();

#endif

#ifdef CAPABLE433
  pinMode(pinSender433, OUTPUT);
  pinMode(pinReceiver433, INPUT);
  mySwitch.enableTransmit(pinSender433);
  mySwitch.enableReceive(pinReceiver433);  // Receiver on interrupt 0 => that is pin #2
#endif



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
      client.subscribe(topicSendIR);
      client.subscribe(topicSend433);
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

#ifdef CAPABLEIR

   // Check if the IR code has been received.
  if (irrecv.decode(&results, &save)) {
    dumpCode(&results);           // Output the results as source code
    sendMQTTIR(&results);  
  }
#endif

#ifdef CAPABLE433
  if (mySwitch.available()) {
    
    int value = mySwitch.getReceivedValue();
    
    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      Serial.print("Received 433");
      code433= mySwitch.getReceivedValue(); 
      Serial.print(code433);
      Serial.print(" / ");
      bits433=mySwitch.getReceivedBitlength();
      Serial.println(bits433);
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() );
      sendMQTT433(code433,bits433); 
    }

    mySwitch.resetAvailable();
  }

#endif

  //Serial.println("no envio nada");
  delay(loopDelay);
  
    
}

void cleanAnswerBuffer(){
   for(int itr=0; itr<answerBufferSize; itr++){
    answerBuffer[itr]=0;
    
    }
}

