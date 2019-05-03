
#include "MIDIUSB.h"
#include <Adafruit_DotStar.h>
#include "pitchToFrequency.h"

// M0 Trinket Synth - Exercise 01
// Connectivity
//
// Check that we are connecting to Ableton ( or whatever ) and verify that
// we can see note on, note off and CC messages in serial and visualize notes
// as on/off ( make it blink)

// Borrowed Heavily from
// https://github.com/arduino-libraries/MIDIUSB/tree/master/examples/MIDIUSB_buzzer




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


const char* pitch_name(byte pitch) {
  static const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  return names[pitch % 12];
}

int pitch_octave(byte pitch) {
  return (pitch / 12) - 1;
}


void setup() {
  Serial.begin(115200);
  Serial.println("Setting up:");

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP
}


// 0x9
void noteOn(byte channel, byte pitch, byte velocity) {

  logData(0x9,   channel,   pitch,   velocity);

  // Indicate on via dot
  strip.setPixelColor(0, 0x9F0000); // red
  strip.show();
}

// 0x8
void noteOff(byte channel, byte pitch, byte velocity) {

  logData(0x8,   channel,   pitch,   velocity) ;

  // Indicate off via dot
  strip.setPixelColor(0, 0x000000); // red
  strip.show();
}

// 0xB
void controlChange(byte channel, byte control, byte value) {
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



void logData(byte header, byte channel, byte byte2, byte byte3) {

  switch (header) {
    case 0:
      return;


    case 0x9:

      Serial.print("Note On: ");
      Serial.print(pitch_name(byte2));
      Serial.print(pitch_octave(byte2));
      Serial.print(", channel=");
      Serial.print(channel);
      Serial.print(", velocity=");
      Serial.println(byte3);
      break;

    case 0x8:
      Serial.print("Note Off: ");
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
