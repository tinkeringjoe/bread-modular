#include <Arduino.h>

#define PIN_CV1 PIN_PA7
#define PIN_CV2 PIN_PB0

void setup() {
  // DAC0 setup via the PA6 pin
  VREF.CTRLA |= VREF_DAC0REFSEL_4V34_gc; //this will force it to use VDD as the VREF
  VREF.CTRLB |= VREF_DAC0REFEN_bm;
  DAC0.CTRLA = DAC_ENABLE_bm | DAC_OUTEN_bm;
  DAC0.DATA = 0;

  analogReference(VDD);
  pinMode(PIN_CV1, INPUT);
  pinMode(PIN_CV2, INPUT);
}

void mode_add() {
  float cv1 = analogRead(PIN_CV1) / 1023.0;
  float cv2 = analogRead(PIN_CV2) / 1023.0;
  DAC0.DATA = constrain((cv1 + cv2), 0.0, 1.0) * 255;
}

void loop() {
  mode_add();
}
