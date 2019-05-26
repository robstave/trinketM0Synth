void setColor(byte coreIndex) {

  if (coreArray[coreIndex].lfoSwitchState == ALWAYS_ON) {
    coreArray[coreIndex].color = 0xFF;
  } else {
    if (coreArray[coreIndex].lfoSwitchState == LFO_ON) {

      if (coreArray[coreIndex].lfoState % 2 == 0) {
        coreArray[coreIndex].color = 0x00;
      } else {
        coreArray[coreIndex].color = 0xFF;
      }
    } else {
      // LFO CODE.  Always off
      coreArray[coreIndex].color = 0x00;
    }
  }
}
