# M0_Simple_Synth_07

Modest improvments on 06.


![Capture1](https://github.com/robstave/trinketM0Synth/blob/master/nandsynth/M0_Simple_Synth_07/images/circuit3.PNG)

Added a knob to control the LFO entirely.  
  - NO LFO, always on
  - LFO ON
  - Cut the Circuit

Replaced the NAND summer with a Function that can be either
 
  - NAND
  - XOR
  - Mixer


Previously, we were flipping the output bit from 0 to 100% percent, but really, its an analog bit, so
we can actually sum the two values.  You will note that the volume drops a little in this case too.

By this point, if you are following the code, you will note that its starting to have a lot of variables and getting messy.  I will do a refactor in the next one.

# Example

![example](https://www.youtube.com/watch?v=09VcLD3l2AE)
