#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <MIDI.h>
#include "tones.h"
#include "utils.h"

#define GATE_PIN PIN_PA7

// Create a MIDI object
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

void handleNoteOn(byte channel, byte note, byte velocity) {
  const int freq = midiToFrequency(note);
  setTone1Frequency(freq);

  const int detunedFreq = midiToFrequency(note - 12);
  setTone2Frequency(detunedFreq);

  digitalWrite(GATE_PIN, HIGH);
}

void handleNoteOff(byte channel, byte note, byte velocity) {
  digitalWrite(GATE_PIN, LOW);
}

void setup() {
  // define the gate pin
  pinMode(GATE_PIN, OUTPUT);
  digitalWrite(GATE_PIN, LOW);

  // Setup tones
  setupTone1();
  setupTone2();

  // Setup Serial Comm
  Serial.begin(31250);
  
  // Initialize the MIDI library
  // Listen to only the channel 1
  MIDI.begin(1);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  Serial.println("MCO Started!");

  setTone1Frequency(20);
  setTone2Frequency(20);
}

void loop() {
  // parse incoming MIDI messages
  MIDI.read();
}
