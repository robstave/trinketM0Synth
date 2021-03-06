# trinketM0Synth - A Journey into Noise

Experiments into MIDI controllers and sound generation using the Adafruit M0 Trinket

## Objective

Learn how to connect the ADafruit **M0 Trinket** to Midi controllers. In baby steps!

Generally, I have been inspired by the Lunetta side of things, so I will be focusing on Squarewave/8-bit/Noise projects.  But if I can leave a trail of mini-discoveries that others can run with, that is all that matters.

I was inspired by:
 - A stack of ESP8266 boards that were cheaper to buy than a * bucks  Latte.  What to do with them?
 - [How to control anything on arduino using midi](https://www.partsnotincluded.com/arduino/how-to-control-anything-on-arduino-using-midi/)

I spent a whole night just getting a light to blink on an ESP8266 using my [Arturia Beatstep](https://www.arturia.com/beatstep/overview).  I was successful, but also learned that I would ultimately be stuck with all kinds of middleware like a Midi Loop app AND a USB midi serial converter to make that work.  Perhaps there is an easier way ( HID bootloader on ESP8266)?

I also had a Trinket M0 (SAMD21) sitting around and it apparently did not have those hang ups. So, here we are.  

So, in this repo, I will be learning how to control a trinket M0 from a midi controller to make skin crawling bleeps, bloops and blink a few LEDs on the way.

## Setup

This github does assume that you have an M0 Trinket and you can at least get it to blink and run some of the basic included sketches from Adafruit. There should be plenty of resources to get that up and running.  

https://learn.adafruit.com/adafruit-trinket-m0-circuitpython-arduino/arduino-ide-setup

Perhaps run a few of the initial sketches there as well.  There should be one that makes the on board LED (DOT) blink.  I will be using that.

Once you get that up, you will need to install the MIDIUSB.h library.

https://github.com/arduino-libraries/MIDIUSB
http://blog.stekgreif.com/?p=699
https://tigoe.github.io/SoundExamples/midiusb.html

Finally, hardware.  I am using the Ableton LITE to push Midi from a midi controller to the trinket.  You might be able to get away with any other DAW or perhaps you can get both things to just talk to each other over a USB bus.  I did not try that.  Lite usually comes free with most decent controllers anyways these day.  Just bought Lite and its totally worth it.

You will need a midi controller with knobs/sliders or some kind of CC controller. 

I use the [Arturia Beatstep](https://www.arturia.com/products/hybrid-synths/beatstep/overview)

If you just have a DAW, you can git away with a virtual synth too.  Changing knobs on a vst synth will forward the CCs to the trinket if set up correctly.  Or you can sequence the midi directly in the DAW and push that too. 

 
## NAND Synth 

Low res simulation of a ![NAND Synth](https://github.com/robstave/trinketM0Synth/tree/master/nandsynth) 
 Based on interrupts and counters.
 


## DDS Synth Drone

Better frequency resolution using DDS

## Midi Controller

Build a Looper pedal controller ( or whatever you want buttons to do)




