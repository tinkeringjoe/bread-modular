#include <Arduino.h>
#include <SoftwareSerial.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "utils.h"
#include "SimpleMIDI.h"
#include "CustomOscillator.h"

#define GATE_PIN PIN_PA7
#define LOGGER_PIN_TX PIN_PB4

SimpleMIDI MIDI;
SoftwareSerial logger = SoftwareSerial(-1, LOGGER_PIN_TX);

const int8_t sineTable[256] = {
0, 3, 6, 9, 12, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 49, 51, 54, 57, 60,
63, 65, 68, 71, 73, 76, 78, 81, 83, 85, 88, 90, 92, 94, 96, 98, 100, 102, 104,
106, 107, 109, 111, 112, 113, 115, 116, 117, 118, 120, 121, 122, 122, 123, 124,
125, 125, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 126, 126, 126, 125,
125, 124, 123, 122, 122, 121, 120, 118, 117, 116, 115, 113, 112, 111, 109, 107,
106, 104, 102, 100, 98, 96, 94, 92, 90, 88, 85, 83, 81, 78, 76, 73, 71, 68, 65,
63, 60, 57, 54, 51, 49, 46, 43, 40, 37, 34, 31, 28, 25, 22, 19, 16, 12, 9, 6, 3,
0, -3, -6, -9, -12, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46, -49,
-51, -54, -57, -60, -63, -65, -68, -71, -73, -76, -78, -81, -83, -85, -88, -90,
-92, -94, -96, -98, -100, -102, -104, -106, -107, -109, -111, -112, -113, -115,
-116, -117, -118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126,
-126, -127, -127, -127, -127, -127, -127, -127, -126, -126, -126, -125, -125,
-124, -123, -122, -122, -121, -120, -118, -117, -116, -115, -113, -112, -111,
-109, -107, -106, -104, -102, -100, -98, -96, -94, -92, -90, -88, -85, -83, -81,
-78, -76, -73, -71, -68, -65, -63, -60, -57, -54, -51, -49, -46, -43, -40, -37,
-34, -31, -28, -25, -22, -19, -16, -12, -9, -6, -3,
 };

CustomOscillator oscillator(sineTable, 256, 10000);

void setupTimer() {
    TCB0.CTRLA |= TCB_ENABLE_bm; // counting value

    // Uses 20Mhz clock & divide it by for here
    // So, 1 tick is 0.1us
    TCB0.CTRLA |= TCB_CLKSEL_CLKDIV2_gc; // clock div by 2

    // The intruppt is runnign at every 100us
    TCB0.CCMP = 1000; // Set the compare value
  
    // Enabling inturrupts
    TCB0.CTRLB |= TCB_CNTMODE_INT_gc; // Timer interrupt mode (periodic interrupts)
    TCB0.INTCTRL = TCB_CAPT_bm; // Enable Capture interrupt
    sei();
}

// TCB0 Interrupt Service Routine
ISR(TCB0_INT_vect) {
  // Output sine wave to DAC
  DAC0.DATA = oscillator.next() + 127;

  // Clear the interrupt flag∫
  TCB0.INTFLAGS = TCB_CAPT_bm;
}

void setup() {
  // define the gate pin
  pinMode(GATE_PIN, OUTPUT);
  digitalWrite(GATE_PIN, LOW);

  // Setup Logger
  pinMode(LOGGER_PIN_TX, OUTPUT);
  logger.begin(9600);
  
  // Set the analog reference for ADC with supply voltage.
  analogReference(VDD);
 
  // DAC0 setup for sending velocity via the PA6 pin
  VREF.CTRLA |= VREF_DAC0REFSEL_4V34_gc; //this will force it to use VDD as the VREF
  VREF.CTRLB |= VREF_DAC0REFEN_bm;
  DAC0.CTRLA = DAC_ENABLE_bm | DAC_OUTEN_bm;
  DAC0.DATA = 0;

  // Setup MIDI
  MIDI.begin(31250);

  setupTimer();
  oscillator.setFrequency(1000);

  logger.println("MCO 0.2.0 Started!");
}

void loop() {
  // parse incoming MIDI messages
   if (MIDI.read()) {
      uint8_t type = MIDI.getType();
      uint8_t channel = MIDI.getChannel();
      uint8_t data1 = MIDI.getData1();
      uint8_t data2 = MIDI.getData2();

      // Debug output
      if (type == MIDI_NOTE_ON) {
        float freq = midiToFrequency(data1);
        oscillator.setFrequency(freq);   
      } else if (type == MIDI_NOTE_OFF) {
          
      } else if (type == MIDI_CONTROL_CHANGE) {
          
      } else {
          logger.print("Unknown | ");
      }
  }
}
