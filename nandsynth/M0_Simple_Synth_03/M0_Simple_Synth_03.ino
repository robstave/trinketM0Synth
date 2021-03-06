
#include "MIDIUSB.h"
#include <Adafruit_DotStar.h>
#include "pitchToFrequency.h"

// M0 Trinket Synth - Exercise 03
// Create a Simple LFO controlled by CC
//


// Setup Onboard Dot
#define NUMPIXELS 1 // Number of LEDs in strip

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


uint8_t counter = 0;

uint8_t lfoCCValue = 100;


// Set timer TC4 to call the TC4_Handler every 120ms
// taken from
// http://forum.arduino.cc/index.php?topic=599151.0
void setupTimer4() {
  // Feed GCLK0 to TC4 and TC5
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |                 // Enable GCLK0 to TC4 and TC5
                      GCLK_CLKCTRL_GEN_GCLK0 |             // Select GCLK0
                      GCLK_CLKCTRL_ID_TC4_TC5;             // Feed the GCLK0 to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY);                       // Wait for synchronization

  TC4->COUNT16.CC[0].reg = 5624;                           // Set the TC4 CC0 register as the TOP value in match frequency mode
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
}


void TC4_Handler()                                         // Interrupt Service Routine (ISR) for timer TC4
{
  // Check for overflow (OVF) interrupt
  if (TC4->COUNT16.INTFLAG.bit.OVF && TC4->COUNT16.INTENSET.bit.OVF)
  {
    // Put your timer overflow (OVF) code here....
    counter++;

    if (counter % 2 == 0) {
      strip.setPixelColor(0, 0, 0, 0); //set the pixel colors

    } else {

      strip.setPixelColor(0, 0xFF, 0, 0); //set the pixel colors
    }
    strip.show();

    // Serial.print(" up:");
    // Serial.println(counter);


    TC4->COUNT16.INTFLAG.reg = TC_INTFLAG_OVF;             // Clear the OVF interrupt flag
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println("Setting up:");

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP

  setupTimer4();
}

 
 
# define maxLFO  6000
# define minLFO 60000

// 0xB
void controlChange(byte channel, byte control, byte value) {

  if (lfoCCValue == value){
    return;
  }
  lfoCCValue = value;
  uint16_t interruptSpeed;
  
  interruptSpeed = map(value, 0, 127, minLFO, maxLFO);

  TC4->COUNT16.CC[0].reg = interruptSpeed;

  Serial.print("i:");

  Serial.println(interruptSpeed);


  // intensity = value;
  // strip.setBrightness(intensity);
  logData(0xB,   channel,   control,   value) ;
}


void loop() {


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


const char* pitch_name(byte pitch) {
  static const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  return names[pitch % 12];
}

int pitch_octave(byte pitch) {
  return (pitch / 12) - 1;
}


void logData(byte header, byte channel, byte byte2, byte byte3) {

  switch (header) {
    case 0:
      return;


    case 0x9:

      Serial.print("Note On: ");
      Serial.println(byte2);
      Serial.print(pitch_name(byte2));
      Serial.print(pitch_octave(byte2));
      Serial.print(", channel=");
      Serial.print(channel);
      Serial.print(", velocity=");
      Serial.println(byte3);
      break;

    case 0x8:
      Serial.print("Note Off: ");
      Serial.println(byte2);
      Serial.print(pitch_name(byte2));
      Serial.print(pitch_octave(byte2));
      Serial.print(", channel=");
      Serial.print(channel);
      Serial.print(", velocity=");
      Serial.println(byte3);

      break;

    case 0xB:
      Serial.print("Control change: control=");
      Serial.print(byte2);
      Serial.print(", value=");
      Serial.print(byte3);
      Serial.print(", channel=");
      Serial.println(channel);
      break;

    default:
      Serial.print("Unhandled MIDI message: ");
      Serial.print(header, HEX);
      Serial.print("-");
      Serial.print(channel, HEX);
      Serial.print("-");
      Serial.print(byte2, HEX);
      Serial.print("-");
      Serial.println(byte3, HEX);

      break;

  }
}


// Pretty sequence to know it started up
void initSequence() {
  strip.setPixelColor(0, 0x1F0000); // red
  strip.show();
  delay(500);

  strip.setPixelColor(0, 0x001F00); // green
  strip.show();
  delay(050);

  strip.setPixelColor(0, 0x00001F); // blue
  strip.show();
  delay(500);

  strip.setPixelColor(0, 0x000000); // blue
  strip.show();
}
