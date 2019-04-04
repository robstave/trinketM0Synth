// Pretty sequence to know it started up.  Just Call it from init or the loop code (assuming your loop has a while-true loop in there as well)

// This is optional.  But I like to see a little sequence indicator to know that it uploaded or restarted.

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
