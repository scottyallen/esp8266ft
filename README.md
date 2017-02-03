This sketch should work to flash the noisebridge lightstrip equipment, at the
time of writing the square noise table at square.noise and the bookshelf lights
at bookcase.noise

Easy flashing instructions in Setup, Program, and Test sections below:

_Setup_
* get a system.  Linux osX or windows based.  Anything with usb ports.  
* get a good usb cable.  dont get a flakey cable. test cable by connecting the
  nodemcu to your system with screen\* while jerking it around to see if screen
  drops away back to shell (\* `screen /dev/ttyUSBX 115200`). No flakey cables!
* use arduino version 1.6.4 or higher (for the ease of the built-in board manager)

* re arduino:
follow sparkfun instructions for adding esp8266 based board defs to arduino 
https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon
( http://arduino.esp8266.com/stable/package_esp8266com_index.json )

* \#use the nodeMCU 0.9 ESP-12 Module from the Tools>borads menu pick in Arduino (unverified)
* use the nodeMCU 1.0 ESP-12E module option from the Tools>boards menu pick in Arduino (worked)

_Program_
* Disconnect the nodeMCU from the flaschen lighten.
* Connect it to your system.
* The usb-uart bridge on the nodeMCU should make a serial prot show up on your system, or you
may need drivers for it to work.
* For the current installed devices, the usb-uart bridge chip is a silabs cp210x/cp210x. 
* Once you have a good serial connection btwn your system and the nodeMCU (I use screen), open
arduino and send the included esp8266ft sketch from arduino IDE using baud rate
115200 and whatever usb port showed up when the usb-uart bridge was connected
to your system.

_Test_
* connect serial (see the setup section above) to your system
* confirm from serial output


TODOS:

* code should progress to demo mode even without making a wifi connection. timeout?
* Enhance serial outputs.. 
* Add a improved reset.
* Add sanity check (does it tell you its working when it is disconnected from the lightstrip? When its disconnected from the serial?)
* Add watchdog for flakey esp8266 wifi stack -- another uC
* build alternative flashing decoupled from arduino
* submodule/pin esp8266 libraries to the repo so the project has a better chance
  of building.  the esp8266 gcc/heap/memory map defs are a moving target and the
  longer sketches may not compile as versions change.
