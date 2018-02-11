This project allows to control ir remote devices (like sound systems or TVs) using MQTT messages through an arduino wireless device.


It is based on the following arduino libraries:
[ESP8266Wifi]() For use an esp8266 device
[PubSub Client](https://github.com/knolleary/pubsubclient) For connecting to MQTT broker.
[ArduinoJSON]() For reading and generating messages
[IRremote]() For receiving and sending infrared signals


Usage:

 * You should create a wifi_conf.c file with your wifi essid and password, like showed in file beginning.
 * You should define where is your mqtt broker, like mosquitto, as it is shown in the beginning of the file. By default MQTT broker is wifi gateway;
 * Flash your device using an arduino like toolkit and the previous libraries.

 * You can receive the code of your remotes subscribing to topic "receivedIR".
 * You can send signals by resending the json received in the reception topics to topics "sendIR".



Electronics:

 * For infrarred sending, it is used an ir led with a 220 resistor connected pin 12.
 * For infrarred receving, it is used a 38KHz phototransitor connected to pin 14.

