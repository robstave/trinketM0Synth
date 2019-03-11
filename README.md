# trinketM0Synth - A Journey into Noise

Experiments into M0 using the Adafruit M0 Trinket

## Objective

Learn how to connect the M0 Trinket to Midi controller from the baby steps of making things blink to a instrument.

Generally, I have been inspired by the Lunetta side of things, so I will be focusing on Squarewave/8-bit/Noise projects.  But if I can leave a trail of mini-discoveries that others can run with, that is all that matters.

I was inspired by:
 - A stack of ESP8266 boards that were cheaper to buy than a *bucks  Latte.  What to do with them?
 - [How to control anything on arduino using midi](https://www.partsnotincluded.com/arduino/how-to-control-anything-on-arduino-using-midi/)

I spent a whole night just getting a light to blink on an ESP8266 using my [Arturia Beatstep](https://www.arturia.com/beatstep/overview).  I was successful, but also learned that I would ultimately be harnessed to a Midi Loop app AND a USB midi serial converter to make that work.  Perhaps there is an easier way ( HID bootloader on ESP8266)?

I also had a Trinket M0 ( SAMD21) sitting around and it apparently did not have those hang ups. So, here we are. ( Btw, if your fine with controlling hardware from Ableton on your pc, there is nothing wrong with the ESP8266 route...160MHz...you bet!).

So, in this repo, I will be learning how to control a trinket M0 from a midi controller to make skin crawling bleeps, bloops and blink a few LEDs on the way.