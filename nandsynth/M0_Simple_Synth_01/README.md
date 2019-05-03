# M0_Simple_Synth_01

Verify that you are getting Midi messages from a Midi controller.

You will need to have the Trinket Library installed.

https://learn.adafruit.com/adafruit-trinket-m0-circuitpython-arduino/arduino-ide-setup

Perhaps run a few of the initial sketches there as well.  There should be one that makes the on board LED (DOT) blink.

You will need to have the Dot Star library installed into you IDE and we are using the
MIDIUSB.h Library.

https://github.com/arduino-libraries/MIDIUSB
http://blog.stekgreif.com/?p=699
https://tigoe.github.io/SoundExamples/midiusb.html

## Setup

You will need some software to actually connect to the Trinket.  In my case, I am using a Ableton Lite.  Both the Controller ( Arturia Beat STep ) and the trinket are connected to my PC (Windows) via USB.

Ableton is set up so that it recieves ( and broadcasts ) the midi.  Because of this, I can tell if the messages are being sent from the controller.

So I loaded up ableton, hooked them both up and amazingly...M0 Showed up as something I could work with

I set the input as my control surface and the output as MO AND you need the monitor to work.

![Capture1](https://github.com/robstave/trinketM0Synth/blob/master/nandsynth/M0_Simple_Synth_01/images/Capture1.PNG)



![Capture](https://github.com/robstave/trinketM0Synth/blob/master/nandsynth/M0_Simple_Synth_01/images/Capture.PNG)

 

The code has three basic handlers

- Note On - Detect a note
- Note Off - Detect a note release
- Control Change - a knob was turned

All it does is capture the message and log it.

Note that the Logging is a little different.  IF you are adapting this for Arduino ( DUE or something legit ) you may need to change the serial code.


