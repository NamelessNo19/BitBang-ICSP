BitBang-ICSP
============

An **experimental** ICSP software using the Arduino ( **NEW:** Raspberry Pi) platform.

Do not use!
-----------
Unless you are me or **really** know what you are doing.

Rarely Asked Questions
----------------------

##### BitBang? Like Cybersex?
[Far from it](http://en.wikipedia.org/wiki/Bit_banging). 

##### And what about ICSP?
[In Circuit Serial Programming](http://en.wikipedia.org/wiki/In_Circuit_Serial_Programming_%28ICSP%29). 
If you don't know what this is, you probably don't need this software.

##### You really need to fix the code layout.
I know. But that's no question.

##### What MCUs are supported?
 * [Microchip PIC18F4550](http://www.microchip.com/wwwproducts/Devices.aspx?dDocName=en010300)
 * [Microchip PIC18F2550](http://www.microchip.com/wwwproducts/Devices.aspx?dDocName=en010280)
 * [Atmel ATTiny13A](http://www.atmel.com/devices/attiny13a.aspx) (Barely.)

##### How do I use this?
Like I said before: __Don't.__ Otherwise you might blow up your MCU / Arduino / Raspberry Pi / home.

##### I don't care. Can I wire the PIC directly to my Arduino / Raspberry Pi?
 * __Arduino__ : Maybe. I still highly recommend using some kind of overcurrent protection. Especially the Vpp-Pin migth draw more power than the Arduino can supply.
 * __Raspberry Pi__ : No, the Raspberry Pi GPIO interface operates at 3.3V, the PIC requires 5V. You need at least one bidirectional level converter and three
 unidirectional 3.3V -> 5V level converters.

##### Are you in any way affiliated to Microchip Technology Inc. or Atmel Corporation?
Not at all.
