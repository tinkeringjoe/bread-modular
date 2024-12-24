#include <Arduino.h>
#include <SoftwareSerial.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <MIDI.h>
#include "utils.h"

#define GATE_PIN PIN_PA7
#define LOGGER_PIN_TX PIN_PB4

SoftwareSerial logger = SoftwareSerial(-1, LOGGER_PIN_TX);

// Create a MIDI object
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

void handleNoteOn(byte channel, byte note, byte velocity) {
  const int freq = midiToFrequency(note);
  digitalWrite(GATE_PIN, HIGH);

  logger.printf("NOTE ON: %d\n", note);

  // Send the velocity out via PA6
  DAC0.DATA = (velocity / 127.0) * 255;
}

void handleNoteOff(byte channel, byte note, byte velocity) {
  digitalWrite(GATE_PIN, LOW);

  logger.printf("NOTE OFF: %d\n", note);

  // Reset the velocity to 0.
  DAC0.DATA = 0;
}

void setupTone1() {
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
int val = 0;
ISR(TCB0_INT_vect) {
  if (val == 0) {
    val = 1;
    DAC0.DATA = 0;
  } else {
    val = 0;
    DAC0.DATA = 255;
  }

  // Clear the interrupt flag
  TCB0.INTFLAGS = TCB_CAPT_bm;
}

void setup() {
  // define the gate pin
  pinMode(GATE_PIN, OUTPUT);
  digitalWrite(GATE_PIN, LOW);

  // Setup Serial Comm
  Serial.begin(31250);

  // Setup Logger
  pinMode(LOGGER_PIN_TX, OUTPUT);
  logger.begin(9600);
  
  // Initialize the MIDI library
  // Listen to only the channel 1
  MIDI.begin(1);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  logger.println("MCO 0.2.0 Started!");


  // Set the analog reference for ADC with supply voltage.
  analogReference(VDD);
 
  // DAC0 setup for sending velocity via the PA6 pin
  VREF.CTRLA |= VREF_DAC0REFSEL_4V34_gc; //this will force it to use VDD as the VREF
  VREF.CTRLB |= VREF_DAC0REFEN_bm;
  DAC0.CTRLA = DAC_ENABLE_bm | DAC_OUTEN_bm;
  DAC0.DATA = 0;

  setupTone1();
}

void loop() {
  // parse incoming MIDI messages
  MIDI.read();
}
