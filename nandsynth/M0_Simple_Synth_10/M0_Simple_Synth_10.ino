
#include "MIDIUSB.h"
#include <Adafruit_DotStar.h>
/*
    #include "pitchToFrequency.h"
*/

/**
   M0 Trinket Synth - Exercise 10

   Two Squarewave oscillator(2 Cores) with 3 CCs each
    - HfCC - CC to control pitch
    - LfoCC - CC to control LFO Frequency
    - LfoOnCC - CC to turn core on/off/LFO  so its always on, always off or squarewave LFO


   CC for the mixer - Toggle Mixer to be sum/nand/xor
   So we had up to 7 knobs.


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

# define ALWAYS_ON 1
# define LFO_ON 2
# define ALWAYS_OFF 3

# define CC_LFO1 10
# define CC_HF1 74
# define CC_LFO_STATE1 71
# define CC_TRILL1 44

# define CC_LFO2 114
# define CC_HF2 18
# define CC_LFO_STATE2 19
# define CC_TRILL2 45

# define CC_MIX_MODE 7

volatile uint8_t mix_mode_value = 10;  // default on at 50/50


typedef struct CoreState {
  uint8_t lfoCC;
  uint32_t lfoCounter;
  uint8_t lfoState;  // counts each time the compare value is set.
  uint8_t lfoCounterCompare;


  uint8_t hfCC;
  uint32_t hfCounter;
  uint8_t hfState; // counts each time the compare value is set.  Lowest bit is on/off
  uint8_t hfCounterCompare;
  boolean doTrill;
  uint8_t hfTrill; // modulation value to add to hfValueCompare


  uint8_t lfoOnCC;
  uint8_t lfoSwitchState;
  uint8_t color;
};


// using global memory.  You could use local variables in loop as well if your
// good with pointers.

// Initialize with a random speed and silence count
volatile CoreState coreArray[] = {
  {
    //   LFO CC = 10;
    // initialize at 91
    CC_LFO1, 0, 0, random(maxLFO, minLFO) ,

    //   HF CC = 74;
    CC_HF1, 0, 0, random(maxHF, minHF), false, 0,

    //   LFO STATE CC = 71;
    CC_LFO_STATE1, LFO_ON, 0x00
  },
  // CORE 2
  {
    //   LFO CC = 114;
    // initialize at 130
    CC_LFO2, 0, 0, random(maxLFO, minLFO),

    //   LFO CC = 18;
    CC_HF2, 0, 0, random(maxHF, minHF), false, 0,

    //   LFO ON CC = 19;
    CC_LFO_STATE2, LFO_ON, 0x00
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

    if (coreArray[CORE1].lfoSwitchState == ALWAYS_ON) {
      // LFO CODE.  Always on
      coreArray[CORE1].lfoState = 127;
    } else {
      if (coreArray[CORE1].lfoSwitchState == LFO_ON) {
        // LFO CODE.  When we hit the counter, flip the state bool (and the color of the light)

        if (coreArray[CORE1].lfoCounter > coreArray[CORE1].lfoCounterCompare) {
          coreArray[CORE1].lfoState++;
          coreArray[CORE1].lfoCounter = 0;
        }
        coreArray[CORE1].lfoCounter++;
      } else {
        // LFO CODE.  Always off
        coreArray[CORE1].lfoState = 0;
      }
    }

    if (coreArray[CORE2].lfoSwitchState == ALWAYS_ON) {
      // LFO CODE.  Always on
      coreArray[CORE2].lfoState = 127;
    } else {
      if (coreArray[CORE2].lfoSwitchState == LFO_ON) {
        // LFO CODE.  When we hit the counter, flip the state bool (and the color of the light)

        if (coreArray[CORE2].lfoCounter > coreArray[CORE2].lfoCounterCompare) {
          coreArray[CORE2].lfoState++;
          coreArray[CORE2].lfoCounter = 0;
        }
        coreArray[CORE2].lfoCounter++;
      } else {
        // LFO CODE.  Always off
        coreArray[CORE2].lfoState = 0;
      }
    }

    TC4->COUNT16.INTFLAG.reg = TC_INTFLAG_OVF;             // Clear the OVF interrupt flag
  }
}

/*
   High Frequency Interrupt
   This is the interrupt that controls the actual signal

   Frequency is determined by countdown timers.  The Larger the number, the lower the freq.
   LFO is checked here as well as basically an on/off thing.

   Value is finally mixed with one of three strategies
    - NAND
    - SUM
    - XOR
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

      // Modulate if trill is on.  Really, we are just messing with the counters
      if (coreArray[CORE1].doTrill == true) {
        if (coreArray[CORE1].hfTrill > 8) {
          coreArray[CORE1].hfTrill = 0;
        }
        if (coreArray[CORE1].hfState % 32 == 1) {
          coreArray[CORE1].hfTrill++;
        }
      }
    }
    coreArray[CORE1].hfCounter++;

    boolean value1 = ( (coreArray[CORE1].lfoState % 2 == 1)  && (coreArray[CORE1].hfState % 2 == 1));

    //CORE 2
    if (coreArray[CORE2].hfCounter > (coreArray[CORE2].hfCounterCompare + coreArray[CORE2].hfTrill)) {
      coreArray[CORE2].hfState++;
      coreArray[CORE2].hfCounter = 0;

      // Modulate if trill is on.  Really, we are just messing with the counters
      if (coreArray[CORE2].doTrill == true) {
        if (coreArray[CORE2].hfTrill > 8) {
          coreArray[CORE2].hfTrill = 0;
        }
        if (coreArray[CORE2].hfState % 32 == 1) {
          coreArray[CORE2].hfTrill++;
        }
      }

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


  // Is LFO CC
  if (control == coreArray[CORE1].lfoCC) {


    coreArray[CORE1].lfoCounterCompare = mapMidiLowBudget(value,  minLFO, maxLFO);
    return;
  }

  if (control == coreArray[CORE2].lfoCC) {
    coreArray[CORE2].lfoCounterCompare = mapMidiLowBudget(value,  minLFO, maxLFO);
    return;
  }

  // Is HF CC
  if (control == coreArray[CORE1].hfCC) {
    coreArray[CORE1].hfCounterCompare = mapMidiLowBudget(value, minHF, maxHF);
    return;
  }


  if (control == coreArray[CORE2].hfCC) {
    coreArray[CORE2].hfCounterCompare = mapMidiLowBudget(value, minHF, maxHF);
    return;
  }


  if (control == CC_MIX_MODE) {
    Serial.print("mix_mode_value:");
    Serial.println(value);
    mix_mode_value = value;
  }


  // CC to control LFO
  // 0-50 OFF NO sound
  // 50-100 LFO on
  // 100 + NO LFO always on

  if (control == coreArray[CORE1].lfoOnCC) {
    if (value > 100) {
      coreArray[CORE1].lfoSwitchState = ALWAYS_ON;
    } else {
      if (value  > 50) {
        coreArray[CORE1].lfoSwitchState = LFO_ON;
      } else {
        coreArray[CORE1].lfoSwitchState = ALWAYS_OFF;
      }
    }

  }

  if (control == coreArray[CORE2].lfoOnCC) {
    if (value > 100) {
      coreArray[CORE2].lfoSwitchState = ALWAYS_ON;
    } else {
      if (value  > 50) {
        coreArray[CORE2].lfoSwitchState = LFO_ON;
      } else {
        coreArray[CORE2].lfoSwitchState = ALWAYS_OFF;
      }
    }
  }

}

// 0x9
void noteOn(byte channel, byte pitch, byte velocity) {

  // Channel 0 check is optional.  Im not sure why Im getting stray notes on initialization
  if (channel == 0) {
    if (pitch == CC_TRILL1) {
      Serial.print("Note on 1:");
      Serial.println(pitch);
      coreArray[CORE1].doTrill = true;

    }
    if (pitch == CC_TRILL2) {
      Serial.print("Note on 2:");
      Serial.println(pitch);
      coreArray[CORE2].doTrill = true;
    }
  }
}

// 0x8
void noteOff(byte channel, byte pitch, byte velocity) {
  if (channel == 0) {
    if (pitch == CC_TRILL1) {
      Serial.print("Note off 1:");
      Serial.println(pitch);
      coreArray[CORE1].doTrill = false;
      coreArray[CORE1].hfTrill = 0;


    }
    if (pitch == CC_TRILL2) {
      Serial.print("Note on 2:");
      Serial.println(pitch);
      coreArray[CORE2].doTrill = false;
      coreArray[CORE2].hfTrill = 0;

    }
  }
}


void loop() {

  Serial.println("Enter Loop:");

  initSequence(); // Blinky intro

  while (true) {

    setColor(CORE1);
    setColor(CORE2);
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

      case 0x9:
        noteOn(
          rx.byte1 & 0xF,  //channel
          rx.byte2,        //pitch
          rx.byte3         //velocity
        );

        break;

      case 0x8:
        noteOff(
          rx.byte1 & 0xF,  //channel
          rx.byte2,        //pitch
          rx.byte3         //velocity
        );

        break;

      default:
        // If your curious
        //logData(rx.header,   rx.byte1,   rx.byte2,   rx.byte3) ;

        break;
    }
  }
}
