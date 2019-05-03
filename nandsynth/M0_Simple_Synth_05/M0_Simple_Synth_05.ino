
#include "MIDIUSB.h"
#include <Adafruit_DotStar.h>
#include "pitchToFrequency.h"

// M0 Trinket Synth - Exercise 05
// make a squarewave sound whose pitch is controlled by CC and is gated (on/off) by another CC acting as an LFO.
//
//  Note some of the boilerplate/extra stuff as been moved to other .ino files.  Arduino IDE just combines them
//  with the dir filename.ino first and the remaining in alphabetical order.


// Setup Onboard Dot
#define NUMPIXELS 1 // Number of LEDs in strip

//  The code is just doing the squarewaves at the moment with very simple counters.
//  In the interrupt, we just count the number of ticks here until flipping the bit.

// LFO Counter Bounds.  The higher the number, the longer it takes to count to it, so
// a high frequency is really indicated with a smaller number.
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
uint8_t CC1 = 7;
uint8_t CC2 = 114;

volatile uint16_t counter = 0;
volatile uint8_t state = 0;  // LFO Bit On/Off
volatile uint16_t counterCompare = 100;

volatile uint16_t counterHF = 0;
volatile uint8_t stateHF = 0;  // HF Bit On/Off
volatile uint16_t counterCompareHF = 100;

volatile uint8_t red = 0;  // Store off Color
volatile uint8_t green = 0;

// Not really needed, but basically avoids doing anything on repeat messages
volatile uint8_t lfoCC1Value = 100;
volatile uint8_t lfoCC2Value = 100;


// Set timer TC4 to call the TC4_Handler every 1ms-ish (To Calculate)
// taken from
// http://forum.arduino.cc/index.php?topic=599151.0
void setupTimer4_5() {
  // Feed GCLK0 to TC4 and TC5
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |                 // Enable GCLK0 to TC4 and TC5
                      GCLK_CLKCTRL_GEN_GCLK0 |             // Select GCLK0
                      GCLK_CLKCTRL_ID_TC4_TC5;             // Feed the GCLK0 to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY);                       // Wait for synchronization

  TC4->COUNT16.CC[0].reg = 100;                           // Set the TC4 CC0 register as the TOP value in match frequency mode
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization

  NVIC_SetPriority(TC4_IRQn, 0);    // Set the Nested Vector Interrupt Controller (NVIC) priority for TC4 to 0 (highest)
  NVIC_EnableIRQ(TC4_IRQn);         // Connect TC4 to Nested Vector Interrupt Controller (NVIC)

  TC4->COUNT16.INTFLAG.reg |= TC_INTFLAG_OVF;              // Clear the interrupt flags
  TC4->COUNT16.INTENSET.reg = TC_INTENSET_OVF;             // Enable TC4 interrupts

  TC4->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCSYNC_PRESC |     // Reset timer on the next prescaler clock
                            TC_CTRLA_PRESCALER_DIV1024 |   // Set prescaler to 1024, 48MHz/1024 = 46.875kHz
                            TC_CTRLA_WAVEGEN_MFRQ |        // Put the timer TC4 into match frequency (MFRQ) mode
                            TC_CTRLA_MODE_COUNT16;         // Set the timer to 16-bit mode
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization

  TC4->COUNT16.CTRLA.bit.ENABLE = 1;                       // Enable TC4
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization



  TC5->COUNT16.CC[0].reg = 10;                           // Set the TC5 CC0 register as the TOP value in match frequency mode
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization

  NVIC_SetPriority(TC5_IRQn, 0);    // Set the Nested Vector Interrupt Controller (NVIC) priority for TC5 to 0 (highest)
  NVIC_EnableIRQ(TC5_IRQn);         // Connect TC4 to Nested Vector Interrupt Controller (NVIC)

  TC5->COUNT16.INTFLAG.reg |= TC_INTFLAG_OVF;              // Clear the interrupt flags
  TC5->COUNT16.INTENSET.reg = TC_INTENSET_OVF;             // Enable TC5 interrupts

  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCSYNC_PRESC |     // Reset timer on the next prescaler clock
                            TC_CTRLA_PRESCALER_DIV64 |   // Set prescaler to 64, 48MHz/64 = ???46.875kHz //TODO fix this
                            TC_CTRLA_WAVEGEN_MFRQ |        // Put the timer TC5 into match frequency (MFRQ) mode
                            TC_CTRLA_MODE_COUNT16;         // Set the timer to 16-bit mode
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization

  TC5->COUNT16.CTRLA.bit.ENABLE = 1;                       // Enable TC5
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization
}


void TC4_Handler()                                         // Interrupt Service Routine (ISR) for timer TC4
{
  // Check for overflow (OVF) interrupt
  if (TC4->COUNT16.INTFLAG.bit.OVF && TC4->COUNT16.INTENSET.bit.OVF)
  {
    // Put your timer overflow (OVF) code here....

    if (counter > counterCompare) {

      // LFO CODE.  When we hit the counter, flip the state bool (and the color of the light)

      if (state % 2 == 0) {
        red = 0;
      } else {
        red = 0xFF;
      }

      strip.setPixelColor(0, red, 0, 0); //set the pixel colors

      strip.show();
      state++;
      counter = 0;
    }

    counter++;

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

    if (counterHF > counterCompareHF) {

      if (stateHF % 2 == 0) {
        analogWrite(A0, 0);
      } else {
        if (state % 2 == 0) {
          analogWrite(A0, 255);
        } else {
          analogWrite(A0, 0);
        }
      }
      stateHF++;
      counterHF = 0;
    }
    counterHF++;

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

  if (control == CC1) {
    if (lfoCC1Value == value) {
      return;
    }
    lfoCC1Value = value;
    //counterCompare = map(value, 0, 127, minLFO, maxLFO);
    counterCompare = mapMidiLowBudget(value,  minLFO, maxLFO);

    Serial.print("counterCompare:");
    Serial.println(counterCompare);
  }


  if (control == CC2) {
    if (lfoCC2Value == value) {
      return;
    }
    lfoCC2Value = value;
    //counterCompareHF = map(value, 0, 127, minHF, maxHF);
    counterCompareHF = mapMidiLowBudget(value, minHF, maxHF);

  }

  logData(0xB,   channel,   control,   value) ;  // optinonal
}


void loop() {


Serial.println("Enter Loop:");
    
  initSequence(); // Blinky intro

 

  while (true) {
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
