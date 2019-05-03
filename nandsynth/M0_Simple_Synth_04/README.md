# M0_Simple_Synth_04

This is just a different way to do the LFO.

Basically, rather than changing the interrupt time, we speed up the timer and use a counter to tick off our actual interrupt code.

This sketch is pretty simple once you strip out the boiler plate.

The Interrupt is based on Timer 4.

The timer has a prescaler of 1024 which brings the timer down to  48MHz/1024 = 46.875kHz.

In addition, we are setting the compare to 500

    TC4->COUNT16.CC[0].reg = 500;   // Set the TC4 CC0 register as the TOP value in match frequency mode
 
So that brings our interrupt to 46875/500 = 93.75 hz

Now....on top of that, our code is only flipping bits when the counter is reached

    if (counter > counterCompare) {
      if (state % 2 == 0) {
        strip.setPixelColor(0, 0, 0, 0); //set the pixel colors
      } else {
        strip.setPixelColor(0, 0xFF, 0, 0); //set the pixel colors
      }
      strip.show();
      state++;
      counter = 0;
    }
    counter++;

If the counter was 50, then we would have a frequency of 93.75 / ( 2 * 50) = 9.375 hz

If the counter was 100, then we would have a frequency of 93.75 / ( 2 * 100) = 4.688 hz
( An octave! )
