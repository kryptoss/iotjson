This project allows to read a current sensor YHDC SCT013 30A/1V using MQTT messages through an arduino wireless device (ESP8266).


It is based on the following arduino libraries:
[ESP8266Wifi]() For use an esp8266 device
[PubSub Client](https://github.com/knolleary/pubsubclient) For connecting to MQTT broker.
[ArduinoJSON]() For reading and generating messages


Usage:

 * You should create a wifi_conf.c file with your wifi essid and password, like showed in file beginning.
 * MQTT broker, like mosquitto is suppoused to be in wifi gateway. Otherway you must change it in the code.
 * Flash your device using an arduino like toolkit and the previous libraries.

 * You can receive the measures and configurations subscribing to topic "meters".
 * You can change configuration sending messages with configuration fields changed to topic "controlMeters".


Electronics:

 * For current sensor, a YHDC SCT013 30A/1V sensor
 * It is configured for deep sleep, so you need to connect RST pin to pin GPIO16


