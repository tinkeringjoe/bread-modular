#include <Arduino.h>
#include <SoftwareSerial.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "utils.h"
#include "SimpleMIDI.h"
#include "CustomOscillator.h"
#include "tables/triangle_2048.h"

#define GATE_PIN PIN_PA7
#define LOGGER_PIN_TX PIN_PB4

SimpleMIDI MIDI;
SoftwareSerial logger = SoftwareSerial(-1, LOGGER_PIN_TX);

CustomOscillator oscillator(TRIANGLE2048_DATA, 2048, 10000);

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

bool read = false;
// TCB0 Interrupt Service Routine
ISR(TCB0_INT_vect) {
  read = true;

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
  oscillator.setFrequency(440);

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
        logger.printf("NOTE ON: %d, %d, %ld\n", data1, (int)freq, oscillator.phaseIncrement);
        digitalWrite(GATE_PIN, HIGH);
      } else if (type == MIDI_NOTE_OFF) {
        digitalWrite(GATE_PIN, LOW);
      } else if (type == MIDI_CONTROL_CHANGE) {
          
      } else {
          logger.print("Unknown | ");
      }
  }

  if (read) {
    // Output sine wave to DAC
    DAC0.DATA = oscillator.next() + 127;
    read = false;
  }
}
