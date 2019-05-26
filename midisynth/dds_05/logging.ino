//  Utils to show the key name and Midi logging in the serial port
//
//
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

 
