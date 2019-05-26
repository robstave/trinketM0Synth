
// Set timer TC4 and TC5

// TC4 is set up with a prescaler of 1024 :  48MHz/1024 = 46.875kHz
// The CC0 is set to 100 so ultimately this timer is happening every 469 HZ

// TC5 is much faster at
// 48MHz/64 = 750kHz
//  750kHz/20 = 37,500


// http://forum.arduino.cc/index.php?topic=599151.0
void setupTimer4_5() {
  // Feed GCLK0 to TC4 and TC5
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |                 // Enable GCLK0 to TC4 and TC5
                      GCLK_CLKCTRL_GEN_GCLK0 |             // Select GCLK0
                      GCLK_CLKCTRL_ID_TC4_TC5;             // Feed the GCLK0 to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY);                       // Wait for synchronization

  TC4->COUNT16.CC[0].reg = 100;                           // Set the TC4 CC0 register as the TOP value in match frequency mode
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization

  NVIC_SetPriority(TC4_IRQn, 0);    // Set the Nested Vector Interrupt Controller (NVIC) priority for TC4 to 0 (highest)
  NVIC_EnableIRQ(TC4_IRQn);         // Connect TC4 to Nested Vector Interrupt Controller (NVIC)

  TC4->COUNT16.INTFLAG.reg |= TC_INTFLAG_OVF;              // Clear the interrupt flags
  TC4->COUNT16.INTENSET.reg = TC_INTENSET_OVF;             // Enable TC4 interrupts

  TC4->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCSYNC_PRESC |     // Reset timer on the next prescaler clock
                            TC_CTRLA_PRESCALER_DIV1024 |   // Set prescaler to 1024, 48MHz/1024 = 46.875kHz
                            TC_CTRLA_WAVEGEN_MFRQ |        // Put the timer TC4 into match frequency (MFRQ) mode
                            TC_CTRLA_MODE_COUNT16;         // Set the timer to 16-bit mode
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization

  TC4->COUNT16.CTRLA.bit.ENABLE = 1;                       // Enable TC4
  while (TC4->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization



  TC5->COUNT16.CC[0].reg = 20;                           // Set the TC5 CC0 register as the TOP value in match frequency mode
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization

  NVIC_SetPriority(TC5_IRQn, 0);    // Set the Nested Vector Interrupt Controller (NVIC) priority for TC5 to 0 (highest)
  NVIC_EnableIRQ(TC5_IRQn);         // Connect TC4 to Nested Vector Interrupt Controller (NVIC)

  TC5->COUNT16.INTFLAG.reg |= TC_INTFLAG_OVF;              // Clear the interrupt flags
  TC5->COUNT16.INTENSET.reg = TC_INTENSET_OVF;             // Enable TC5 interrupts

  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCSYNC_PRESC |     // Reset timer on the next prescaler clock
                            TC_CTRLA_PRESCALER_DIV64 |   // Set prescaler to 64, 48MHz/64 = 750 kHz
                            TC_CTRLA_WAVEGEN_MFRQ |        // Put the timer TC5 into match frequency (MFRQ) mode
                            TC_CTRLA_MODE_COUNT16;         // Set the timer to 16-bit mode
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization

  TC5->COUNT16.CTRLA.bit.ENABLE = 1;                       // Enable TC5
  while (TC5->COUNT16.STATUS.bit.SYNCBUSY);                // Wait for synchronization
}
