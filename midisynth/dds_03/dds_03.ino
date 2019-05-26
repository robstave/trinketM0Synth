
#include "avr/pgmspace.h"
#include "tables.h"


// Plays random notes
// Sums two waves and does a little envelope thing


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


volatile int lf_counter;  //
volatile int lf_counter_compare;  //


void TC4_Handler()                                         // Interrupt Service Routine (ISR) for timer TC4
{
  // Check for overflow (OVF) interrupt
  if (TC4->COUNT16.INTFLAG.bit.OVF && TC4->COUNT16.INTENSET.bit.OVF)
  {
    // Put your timer overflow (OVF) code here....

    if ( lf_counter > lf_counter_compare) {
      lf_counter = 0;

      double dfreq = random(200, 1000);                  // initial output frequency = 440 Hz
      tword_m1 = pow(2, 32) * dfreq / refclk; // calulate DDS new tuning word

      dfreq = random(200, 1000);                  // initial output frequency = 440 Hz
      tword_m2 = pow(2, 32) * dfreq / refclk; // calulate DDS new tuning word

    }
    lf_counter++;


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

    double amp = (lf_counter_compare - lf_counter) / lf_counter_compare;  //

    phaccu1 = phaccu1 + tword_m1; // soft DDS, phase accu with 32 bits
    phaccu2 = phaccu2 + tword_m2; // soft DDS, phase accu with 32 bits

    uint32_t icnt1 = phaccu1 >> 24;   // use upper 8 bits for phase accu as frequency information
    // read value fron ROM sine table and send to PWM DAC

    uint32_t icnt2 = phaccu2 >> 24;   // use upper 8 bits for phase accu as frequency information
    // read value fron ROM sine table and send to PWM DAC


    signed int v1 = (signed char)pgm_read_byte_near(SinTable + icnt1) * 
                    (lf_counter_compare - lf_counter) / lf_counter_compare;
    signed int v2 = (signed char)pgm_read_byte_near(SinTable + icnt2)  * 
                    (lf_counter) / lf_counter_compare;


    //  This is where we are summing the signals.  With M0, we get to play with a value from 0 to 1023.
    //  The most we will see adding two signals is 255 or -255.  That gives us some room to amp up the 
    //  signal be for adding the offset ( 2*127)
    int value = ((v1 + v2) * 2)  + 255;
    // we now should have a signal between 0 and 1023

    analogWrite(A0, value);



    TC5->COUNT16.INTFLAG.reg = TC_INTFLAG_OVF;             // Clear the OVF interrupt flag

  }
}

void setup() {

  Serial.begin(115200);
  Serial.println("Setting up");
  //By setting the write resolution to 10, you can use analogWrite() with values between 0 and 1023 to exploit the full DAC resolution
  analogWriteResolution(10);
  lf_counter_compare = 100;
  double dfreq = 440.0;                  // initial output frequency = 440 Hz
  tword_m1 = pow(2, 32) * dfreq / refclk; // calulate DDS new tuning word

  dfreq = 340.0;                  // initial output frequency = 440 Hz
  tword_m2 = pow(2, 32) * dfreq / refclk; // calulate DDS new tuning word
 
  setupTimer4_5();

}

void loop() {

  Serial.println("Enter Loop:");
  while (true) { }
}
