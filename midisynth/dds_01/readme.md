# DDS 01  

Simple tones

We are going to work our way to something that works incrementally.
Mostly, because this is how I learn and I find that if I have to go back to relearn things....its just easier that way.

Start out with a simple sketch that works with TWO timers.  T4 and T5.

One is going pretty fast, and is our DDS ( Direct Digital Synthesis) timer.
Its a 32 bit counter that you shift by 24 bits and use the last 8 bits as an index into your wave table.

The other timer does nothing.

If successful, you should be hearing a tone on the dac pin and a squarewave
on the other outside of audio, but enough to verify the timing of the interrupt.

