// Pretty sequence to know it started up

// This is optional.  But I like to see a little sequence to know that it uploaded or restarted.

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
