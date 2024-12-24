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
}

void loop() {
  // parse incoming MIDI messages
  MIDI.read();
}
