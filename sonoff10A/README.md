This project allows to control a sonoff 10A device


It is based on the following arduino libraries:
[ESP8266Wifi]() For use an esp8266 device
[PubSub Client](https://github.com/knolleary/pubsubclient) For connecting to MQTT broker.
[ArduinoJSON]() For reading and generating messages


Usage:

 * You should create a wifi_conf.c file with your wifi essid and password, like showed in file beginning.
 * MQTT broker, like mosquitto is suppoused to be in wifi gateway. Otherway you must change it in the code.
 * Flash your device using an arduino like toolkit and the previous libraries.

 * You can receive the status and configurations subscribing to topic "switches".
 * You can change configuration sending messages with configuration fields changed to topic "controlSwitches".

 * You must open and solder serial pins inside sonoff device
 * You must burn the arduino bootloader before change de the firmware


Electronics:

 * Sonoff 10A


