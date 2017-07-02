This project allows to control 433Mhz devices (like remote controlled plugs) or ir remote devices (like sound systems or TVs) using MQTT messages through an arduino wireless device.


It is based on the following arduino libraries:
[ESP8266Wifi]() For use an esp8266 device
[PubSub Client](https://github.com/knolleary/pubsubclient) For connecting to MQTT broker.
[ArduinoJSON]() For reading and generating messages
[RC Switch]() For receiving and sending 433Mhz signals
[IRremote]() For receiving and sending infrared signals


Usage:

 * You should create a wifi_conf.c file with your wifi essid and password, like showed in file beginning.
 * You should define where is your mqtt broker, like mosquitto, as it is shown in the beginning of the file.
 * Flash your device using an arduino like toolkit and the previous libraries.

 * You can receive the code of your remotes subscribing to topic "receivedIR" or "received433".
 * You can send signals by resending the json received in the reception topics to topics "sendIR" or "send433".



Electronics:

 * For infrarred sending, it is used an ir led with a 220 resistor connected pin 12.
 * For infrarred receving, it is used a phototransitor connected to pin 14.
 * For sending 433Mhz signals, it is used [cheap emitter](http://www.ebay.com/itm/433Mhz-Wireless-RF-Transmitter-Module-Receiver-Alarm-Super-Regeneration-Arduino-/262123832438?hash=item3d07cc4476:g:lksAAOSwo6lWNwlv) connected to pin 4.
 * For receiving 433Mhz signals, it is used [cheap receiver](http://www.ebay.com/itm/433Mhz-Wireless-RF-Transmitter-Module-Receiver-Alarm-Super-Regeneration-Arduino-/262123832438?hash=item3d07cc4476:g:lksAAOSwo6lWNwlv) connected to pin 5.

433Mhz emitter and received should be powered using 5v, but esp8266 is powered using 3.3v. You can use a 3.3v regulator for esp8266 board.

Those 433Mhz devices have low gain, so their scope is very limited. Set an antenna would increase action circle.     
