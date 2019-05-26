
#include "avr/pgmspace.h"
#include "tables.h"
#include "MIDIUSB.h"

#include "pitchToFrequency.h"

// added midi notes and logs for cc

// Fix Serial for SAMD
#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
// Required for Serial on Zero based boards
#define Serial SERIAL_PORT_USBVIRTUAL
#endif


const double refclk = 35780;
volatile uint32_t phaccu1;   // pahse accumulator
volatile uint32_t tword_m1;  // dds tuning word m

volatile uint32_t phaccu2;   // pahse accumulator
volatile uint32_t tword_m2;  // dds tuning word m

volatile double volumeOsc1 = 1.0;  //  Will define now, but adjust later
volatile double volumeOsc2 = 1.0;  //
volatile double detuneOSc2 = 1.07;  //

 


void TC4_Handler()                                         // Interrupt Service Routine (ISR) for timer TC4
{
  // Check for overflow (OVF) interrupt
  if (TC4->COUNT16.INTFLAG.bit.OVF && TC4->COUNT16.INTENSET.bit.OVF)
  {
    // Put your timer overflow (OVF) code here....
    // Will use in a bit

    TC4->COUNT16.INTFLAG.reg = TC_INTFLAG_OVF;             // Clear the OVF interrupt flag
  }
}

/*
   High Frequency Interrupt
   This is the interrupt that controls the actual signal


*/

void TC5_Handler()                                         // Interrupt Service Routine (ISR) for timer TC4
{
  // Check for overflow (OVF) interrupt
  if (TC5->COUNT16.INTFLAG.bit.OVF && TC5->COUNT16.INTENSET.bit.OVF)
  {

 
    phaccu1 = phaccu1 + tword_m1; // soft DDS, phase accu with 32 bits
    phaccu2 = phaccu2 + tword_m2; // soft DDS, phase accu with 32 bits

    uint32_t icnt1 = phaccu1 >> 24;   // use upper 8 bits for phase accu as frequency information
    // read value fron ROM sine table and send to PWM DAC

    uint32_t icnt2 = phaccu2 >> 24;   // use upper 8 bits for phase accu as frequency information
    // read value fron ROM sine table and send to PWM DAC


    signed int v1 = ((signed char)pgm_read_byte_near(SinTable + icnt1))* volumeOsc1 ;
    signed int v2 = ((signed char)pgm_read_byte_near(SawTable + icnt2) )* volumeOsc2;
 

    int value = ((v1 + v2)  ) + 255;
    // should be between 0 and 1023

    analogWrite(A0, value);



    TC5->COUNT16.INTFLAG.reg = TC_INTFLAG_OVF;             // Clear the OVF interrupt flag

  }
}

void setup() {

  Serial.begin(115200);
  Serial.println("Setting up");
  analogWriteResolution(10);
 tword_m1 = 0;
  tword_m2 = 0;
  setupTimer4_5();
}



// 0xB
void controlChange(byte channel, byte control, byte value) {

  logData(0xB,   channel,   control,   value) ;  // optional


  //if (control == coreArray[CORE2].lfoCC) {
  //  coreArray[CORE2].lfoCounterCompare = mapMidiLowBudget(value,  minLFO, maxLFO);
  //  return;
  //}



}

// 0x9
void noteOn(byte channel, byte pitch, byte velocity) {

  // Channel 0 check is optional.  Im not sure why Im getting stray notes on initialization
  if (channel == 0) {
    Serial.print("Note on 1:");
    Serial.println(pitchFrequency[pitch]);

    double dfreq = pitchFrequency[pitch]  ;                 // initial output frequency = 440 Hz
    tword_m1 = pow(2, 32) * dfreq / refclk; // calulate DDS new tuning word

    // Add second voice as octave below
    tword_m2 = tword_m1 >> 1;

  
  }
}

// 0x8
void noteOff(byte channel, byte pitch, byte velocity) {
  if (channel == 0) {
    Serial.print("Note off 1:");
    Serial.println(pitch);

    tword_m2 = tword_m1 = 0;
   
  }
}


void loop() {

  Serial.println("Enter Loop:");
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
