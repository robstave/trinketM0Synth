
#include "MIDIUSB.h"
#include <Adafruit_DotStar.h>
#include "pitchToFrequency.h"

// M0 Trinket Synth - Exercise 07
// make a squarewave sound whose pitch is controlled by two CC and is gated (on/off) by another two CCs acting as an LFOs.
// Added two on/off switch to turn the LFO off entirely
// Toggle Mixer to be sum/nand/xor
//
//  Note some of the boilerplate/extra stuff as been moved to other .ino files.  Arduino IDE just combines them
//  with the dir filename.ino first and the remaining in alphabetical order.


// Setup Onboard Dot
#define NUMPIXELS 1 // Number of LEDs in strip

//  The code is just doing the squarewaves at the moment with very simple counters.
//  In the interrupt, we just count the number of ticks here until flipping the bit.

// LFO Counters.  minLFO is actually a high number because its a larger tick to
# define maxLFO  40
# define minLFO 500

# define maxHF 70
# define minHF 800


// Here's how to control the LEDs from any two pins:
#define DATAPIN   7
#define CLOCKPIN   8
Adafruit_DotStar strip = Adafruit_DotStar(
                           NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

// Fix Serial for SAMD
#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
// Required for Serial on Zero based boards
#define Serial SERIAL_PORT_USBVIRTUAL
#endif


// Control channels we are listening for.
// Run this and watch the serial port while twisting a knob.  Note the values and set them here and recompile/reload the sketch.
uint8_t CC1HF = 10;   // Control HF 1 pitch
uint8_t CC1LFO = 74;  // Control LFO 1 pitch
uint8_t CC1LFO_ON = 71;  //  LFO 1 on/off state

uint8_t CC2HF = 114; // Control HF 2 pitch
uint8_t CC2LFO = 18;  // Control LFO 2 pitch
uint8_t CC2LFO_ON = 19;//  LFO 2 on/off state

uint8_t MIX_MODE = 7; // Control Mix Mode pitch

volatile uint8_t mix_mode_value = 10;  // default on at 50/50

volatile uint16_t counterLFO1 = 0;
volatile uint8_t stateLFO1 = 0;  // LFO Bit On/Off
volatile uint16_t counterCompareLFO1 = 90;
volatile uint8_t switchLFO1 = 100;  // default on


volatile uint16_t counterLFO2 = 0;
volatile uint8_t stateLFO2 = 0;  // LFO Bit On/Off
volatile uint16_t counterCompareLFO2 = 100;
volatile uint8_t switchLFO2 = 100;  // default on


volatile uint16_t counterHF1 = 0;
volatile uint8_t stateHF1 = 0;  // HF Bit On/Off
volatile uint16_t counterCompareHF1 = 130;

volatile uint16_t counterHF2 = 0;
volatile uint8_t stateHF2 = 0;  // HF Bit On/Off
volatile uint16_t counterCompareHF2 = 100;

volatile uint8_t red = 0;  // Store off Color
volatile uint8_t green = 0;

// Not really needed, but basically avoids doing anything on repeat messages
volatile uint8_t lfoCCLFO1Value = 101;
volatile uint8_t lfoCCLFO2Value = 101;
volatile uint8_t lfoCCHF1Value = 101;
volatile uint8_t lfoCCHF2Value = 101;


void TC4_Handler()                                         // Interrupt Service Routine (ISR) for timer TC4
{
  // Check for overflow (OVF) interrupt
  if (TC4->COUNT16.INTFLAG.bit.OVF && TC4->COUNT16.INTENSET.bit.OVF)
  {
    // Put your timer overflow (OVF) code here....

    if (switchLFO1 > 63) {
      if (counterLFO1 > counterCompareLFO1) {

        // LFO CODE.  When we hit the counter, flip the state bool (and the color of the light)
        if (stateLFO1 % 2 == 0) {
          red = 0;
        } else {
          red = 0xFF;
        }
        stateLFO1++;
        counterLFO1 = 0;
      }
      counterLFO1++;
    } else {
      red = 0xFF;
      stateLFO1 = 1;
    }

    if (switchLFO2 > 63) {
      if (counterLFO2 > counterCompareLFO2) {

        // LFO CODE.  When we hit the counter, flip the state bool (and the color of the light)
        if (stateLFO2 % 2 == 0) {
          green = 0;
        } else {
          green = 0xFF;
        }
        stateLFO2++;
        counterLFO2 = 0;
      }

      counterLFO2++;
    } else {
      green = 0xFF;
      stateLFO2 = 1;
    }

    TC4->COUNT16.INTFLAG.reg = TC_INTFLAG_OVF;             // Clear the OVF interrupt flag
  }
}


void TC5_Handler()                                         // Interrupt Service Routine (ISR) for timer TC4
{
  // Check for overflow (OVF) interrupt
  if (TC5->COUNT16.INTFLAG.bit.OVF && TC5->COUNT16.INTENSET.bit.OVF)
  {
    // Put your timer overflow (OVF) code here....

    // Higher Frequency Interrupt...basically the same thing/
    // We could flip a digital pin, but the board has an analog out.
    //
    // Check the LFO state and turn off, on accordingly.

    int stage1 = 0;

    if (counterHF1 > counterCompareHF1) {
      stateHF1++;
      counterHF1 = 0;
    }
    counterHF1++;

    if (counterHF2 > counterCompareHF2) {
      stateHF2++;
      counterHF2 = 0;
    }
    counterHF2++;

    boolean value1 = ( (stateLFO1 % 2 == 1)  && (stateHF1 % 2 == 1));
    boolean value2 = ( (stateLFO2 % 2 == 1)  && (stateHF2 % 2 == 1));

    if (mix_mode_value < 45) {
      if ( !value1 && !value2) {
        analogWrite(A0, 127);
      } else {
        analogWrite(A0, 0);
      }
    } else if (mix_mode_value >= 45 && mix_mode_value < 100) {



      int  value = 0;
      if (value1 ) {
        value = 63;
      }
      if (value2 ) {
        value = value + 64;
      }

      analogWrite(A0, value);

     
      
  
    } else {
      if ( (value1  && !value2) || (!value1  && value2) ) {
        analogWrite(A0, 127);
      } else {
        analogWrite(A0, 0);
      }
    }



    TC5->COUNT16.INTFLAG.reg = TC_INTFLAG_OVF;             // Clear the OVF interrupt flag

  }
}


void setup() {
  Serial.begin(115200);
  Serial.println("Setting up: Version 05");

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP

  setupTimer4_5();

}


// Wrap Map into a function that is a little 1-logish.  Simple break it a bit in ranges.
// Assumes a midi note in the range of 0-127 and makes it kinda 1-log(x) without a whole lot of
// processing.
//
//
int mapMidiLowBudget(int value, int v1, int v2) {
  int octave = v1 - v2;
  int point1 = v1 - (octave / 2) ;

  if (value < 33) {
    return map(value, 0, 32, v1, point1);
  } else {
    return map(value, 32, 127, point1, v2);
  }
}


// 0xB
void controlChange(byte channel, byte control, byte value) {

  if (control == CC1LFO) {
    if (lfoCCLFO1Value == value) {
      return;
    }
    lfoCCLFO1Value = value;
    counterCompareLFO1 = mapMidiLowBudget(value,  minLFO, maxLFO);

    //Serial.print("counterCompareLFO1:");
    //Serial.println(counterCompareLFO1);
  }

  if (control == CC2LFO) {
    if (lfoCCLFO2Value == value) {
      return;
    }
    lfoCCLFO2Value = value;
    counterCompareLFO2 = mapMidiLowBudget(value,  minLFO, maxLFO);

    //Serial.print("counterCompareLFO2:");
    //Serial.println(counterCompareLFO2);
  }

  if (control == CC1HF) {
    if (lfoCCHF1Value == value) {
      return;
    }
    lfoCCHF1Value = value;
    counterCompareHF1 = mapMidiLowBudget(value, minHF, maxHF);

  }

  if (control == CC2HF) {
    if (lfoCCHF2Value == value) {
      return;
    }
    lfoCCHF2Value = value;
    counterCompareHF2 = mapMidiLowBudget(value, minHF, maxHF);
  }



  if (control == MIX_MODE) {

    Serial.print("mix_mode_value:");
    Serial.println(value);
    mix_mode_value = value;
  }


  if (control == CC1LFO_ON) {
    switchLFO1 = value;
  }

  if (control == CC2LFO_ON) {
    switchLFO2 = value;
  }



  logData(0xB,   channel,   control,   value) ;  // optinonal
}


void loop() {

  Serial.println("Enter Loop:");

  initSequence(); // Blinky intro

  while (true) {

    strip.setPixelColor(0, red, green, 0); //set the pixel colors

    strip.show();


    // Midi packet taken from
    //  midiEventPacket_t rx = MidiUSB.read();
    //
    // First parameter is the event type (0x09 = note on, 0x08 = note off).
    // Second parameter is note-on/note-off, combined with the channel.
    // Channel can be anything between 0-15. Typically reported to the user as 1-16.
    // Third parameter is the note number (48 = middle C).
    // Fourth parameter is the velocity (64 = normal, 127 = fastest).


    midiEventPacket_t rx = MidiUSB.read();
    switch (rx.header) {
      case 0:
        break; //No pending events

      case 0xB:
        controlChange(
          rx.byte1 & 0xF,  //channel
          rx.byte2,        //control
          rx.byte3         //value
        );
        break;

      default:
        // If your curious
        //logData(rx.header,   byte1,   byte2,   byte3) ;

        break;
    }
  }
}
