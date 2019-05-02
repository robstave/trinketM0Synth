
#include "MIDIUSB.h"
#include <Adafruit_DotStar.h>
#include "pitchToFrequency.h"

/**
   M0 Trinket Synth - Exercise 09

   Previously:
   Two Squarewave oscillator(2 Cores) with 3 CCs each
   HfCC - CC to control pitch
   LfoCC - CC to control LFO Frequency
   LfoOnCC - CC to turn core on/off/LFO  so its always on, always off or squarewave LFO
   
   Also another CC for the mixer - Toggle Mixer to be sum/nand/xor

   So we are up to 7 knobs.
   
   At this point, all the values were getting unmanagable, so lets 
   make a struct per core.
  
   
   Added:
   a Vibrato effect
*/

/*
   Note some of the boilerplate/extra stuff as been moved to other .ino files.  Arduino IDE just combines them
   with the dir filename.ino first and the remaining in alphabetical order.

*/


// Setup Onboard Dot
#define NUMPIXELS 1 // Number of LEDs in strip

//  The code is just doing the squarewaves at the moment with very simple counters.
//  In the interrupt, we just count the number of ticks here until flipping the bit.

// LFO Counters.  minLFO is actually a high number because its a larger tick to
# define maxLFO  40
# define minLFO 500

# define maxHF 70
# define minHF 800


# define CORE_COUNT 2
# define PARAMETER_COUNT 13

# define CORE1 0
# define CORE2 1


typedef struct CoreState {
  uint8_t lfoCC;
  uint32_t lfoCounter;
  uint32_t lfoCounterCompare;
  uint8_t lfoState;
  uint8_t lfoValueCompare;


  uint8_t hfCC;
  uint32_t hfCounter;
  uint32_t hfCounterCompare;
  uint8_t hfState;
  uint8_t hfValueCompare;
  uint8_t hfTrill;


  uint8_t lfoOnCC;
  uint8_t lfoSwitchState;

  uint8_t color;


};

// using global memory.  You could use local variables in loop as well if your
// good with pointers.

// Initialize with a random speed and silence count
CoreState coreArray[] = {


  {
    //   HF CC = 10;
    // initialize at 90
    10, 0, 90, 0, 91,

    //   LFO CC = 74;
    74, 0, 90, 0, 91, 0,

    //   LFO ON CC = 71;
    71, 77, 0
  },
  // CORE 2
  {
    //   HF CC = 114;
    // initialize at 130
    114, 0, 130, 0, 91,

    //   LFO CC = 18;
    18, 0, 100, 0, 91, 0,

    //   LFO ON CC = 19;
    19, 77, 0
  },
};


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


uint8_t MIX_MODE_CC = 7;
volatile uint8_t mix_mode_value = 10;  // default on at 50/50


/*
    TC4  LFO Interrupt

    Manipulates the LFO state which is either
    - Always OFF
    - Always ON
    - LFO-ing  Squarewave on off. Note, this is not a bool, but a value, so later we can use it to manipulate Amplitude

    DID I just copy paste the code twice...yes
    Force of habit.  Making it a function only adds a level of indirection to the interrupt code (wasted CPU)
    and 
    Sketch uses 14156 bytes (5%) of program storage space. Maximum is 262144 bytes.

    so its not like we are hurting for program storage
*/

void TC4_Handler()                                         // Interrupt Service Routine (ISR) for timer TC4
{
  // Check for overflow (OVF) interrupt
  if (TC4->COUNT16.INTFLAG.bit.OVF && TC4->COUNT16.INTENSET.bit.OVF)
  {
    // Put your timer overflow (OVF) code here....

    if (coreArray[CORE1].lfoSwitchState > 100) {
      // LFO CODE.  Always on
      coreArray[CORE1].color = 0xFF;
      coreArray[CORE1].lfoState = 127;
    } else {
      if (coreArray[CORE1].lfoSwitchState > 50) {
        // LFO CODE.  When we hit the counter, flip the state bool (and the color of the light)

        if (coreArray[CORE1].lfoCounter > coreArray[CORE1].lfoCounterCompare) {
          coreArray[CORE1].lfoState++;
          if (coreArray[CORE1].lfoState % 2 == 0) {
            coreArray[CORE1].color = 0x00;
            // coreArray[CORE1].hfTrill = 0;  // sync trill
          } else {
            coreArray[CORE1].color = 0xFF;
          }

          coreArray[CORE1].lfoCounter = 0;
        }
        coreArray[CORE1].lfoCounter++;
      } else {
        // LFO CODE.  Always off
        coreArray[CORE1].color = 0x00;
        coreArray[CORE1].lfoState = 0;
      }
    }

    if (coreArray[CORE2].lfoSwitchState > 100) {
      // LFO CODE.  Always on

      coreArray[CORE2].color = 0xFF;
      coreArray[CORE2].lfoState = 127;
    } else {
      if (coreArray[CORE2].lfoSwitchState > 50) {
        // LFO CODE.  When we hit the counter, flip the state bool (and the color of the light)

        if (coreArray[CORE2].lfoCounter > coreArray[CORE2].lfoCounterCompare) {
          coreArray[CORE2].lfoState++;
          if (coreArray[CORE2].lfoState % 2 == 0) {

            coreArray[CORE2].color = 0x00;
          } else {
            coreArray[CORE2].color = 0xFF;
          }

          coreArray[CORE2].lfoCounter = 0;
        }
        coreArray[CORE2].lfoCounter++;
      } else {
        // LFO CODE.  Always off
        coreArray[CORE2].color = 0x00;
        coreArray[CORE2].lfoState = 0;
      }
    }


    TC4->COUNT16.INTFLAG.reg = TC_INTFLAG_OVF;             // Clear the OVF interrupt flag
  }
}

/*
 * High Frequency Interrupt
 * This is the interrupt that controls the actual signal
 * 
 * Frequency is determined by countdown timers.  The Larger the number, the lower the freq.
 * LFO is checked here as well as basically an on/off thing.
 * 
 * Value is finally mixed with one of three strategies
 *  - NAND
 *  - SUM
 *  - XOR
 */

void TC5_Handler()                                         // Interrupt Service Routine (ISR) for timer TC4
{
  // Check for overflow (OVF) interrupt
  if (TC5->COUNT16.INTFLAG.bit.OVF && TC5->COUNT16.INTENSET.bit.OVF)
  {
    // Put your timer overflow (OVF) code here....

    // Higher Frequency Interrupt.

    //CORE 1
    if (coreArray[CORE1].hfCounter > (coreArray[CORE1].hfCounterCompare + coreArray[CORE1].hfTrill)) {
      coreArray[CORE1].hfState++;
      coreArray[CORE1].hfCounter = 0;

      if (coreArray[CORE1].hfTrill > 8) {
        coreArray[CORE1].hfTrill = 0;
      }
      if (coreArray[CORE1].hfState % 32 == 1) {
        coreArray[CORE1].hfTrill++;
      }
    }
    coreArray[CORE1].hfCounter++;

    boolean value1 = ( (coreArray[CORE1].lfoState % 2 == 1)  && (coreArray[CORE1].hfState % 2 == 1));

    //CORE 2
    if (coreArray[CORE2].hfCounter > coreArray[CORE2].hfCounterCompare) {
      coreArray[CORE2].hfState++;
      coreArray[CORE2].hfCounter = 0;
    }
    coreArray[CORE2].hfCounter++;

    boolean value2 = ( (coreArray[CORE2].lfoState % 2 == 1)  && (coreArray[CORE2].hfState % 2 == 1));


    if (mix_mode_value < 45) {

      // NAND MIXER
      if ( !value1 && !value2) {
        analogWrite(A0, 127);
      } else {
        analogWrite(A0, 0);
      }
    } else {

      if (mix_mode_value < 100  ) {

        // SUM mixer

        int  value = 0;
        if (value1 ) {
          value = 63;
        }
        if (value2 ) {
          value = value + 64;
        }

        analogWrite(A0, value);

      } else {

        // XOR Mixer

        if ( (value1  && !value2) || (!value1  && value2) ) {
          analogWrite(A0, 127);
        } else {
          analogWrite(A0, 0);
        }
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

  logData(0xB,   channel,   control,   value) ;  // optional


  if (control == coreArray[CORE1].lfoCC) {

    if (coreArray[CORE1].lfoValueCompare == value) {
      return;
    }
    coreArray[CORE1].lfoValueCompare = value;
    coreArray[CORE1].lfoCounterCompare = mapMidiLowBudget(value,  minLFO, maxLFO);
    return;
  }

  if (control == coreArray[CORE2].lfoCC) {
    if (coreArray[CORE2].lfoValueCompare == value) {
      return;
    }
    coreArray[CORE2].lfoValueCompare = value;
    coreArray[CORE2].lfoCounterCompare = mapMidiLowBudget(value,  minLFO, maxLFO);
    return;
  }

  if (control == coreArray[CORE1].hfCC) {
    if (coreArray[CORE1].hfValueCompare == value) {
      return;
    }
    coreArray[CORE1].hfValueCompare = value;
    coreArray[CORE1].hfCounterCompare = mapMidiLowBudget(value, minHF, maxHF);
    return;
  }


  if (control == coreArray[CORE2].hfCC) {
    if (coreArray[CORE2].hfValueCompare == value) {
      return;
    }
    coreArray[CORE2].hfValueCompare = value;
    coreArray[CORE2].hfCounterCompare = mapMidiLowBudget(value, minHF, maxHF);
    return;
  }


  if (control == MIX_MODE_CC) {
    Serial.print("mix_mode_value:");
    Serial.println(value);
    mix_mode_value = value;
  }


  // CC to control LFO
  // 0-50 OFF NO sound
  // 50-100 LFO on
  // 100 + NO LFO always on

  if (control == coreArray[CORE1].lfoOnCC) {
    coreArray[CORE1].lfoSwitchState = value;
  }

  if (control == coreArray[CORE2].lfoOnCC) {
    coreArray[CORE2].lfoSwitchState = value;
  }

}


void loop() {

  Serial.println("Enter Loop:");

  initSequence(); // Blinky intro

  while (true) {

    strip.setPixelColor(0, coreArray[CORE1].color, coreArray[CORE2].color, 0); //set the pixel colors

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
