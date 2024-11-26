/* Arduino ATTiny 1616 Pin Map

  0 - PA4
  1 - PA5
  2 - PA6
  3 - PA7
  4 - PB5
  5 - PB4
  6 - PB3, RX
  7 - PB2, TX
  8 - PB1
  9 - PB0
  10 - PC0
  11 - PC1
  12 - PC2
  13 - PC3
  14 - PA1
  15 - PA2
  16 - PA3
*/

#include <MIDI.h>
#include <SoftwareSerial.h>

// Create a MIDI object
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

#define PIN_RX 6 // PB3
#define PIN_TX 7 // PB2

// Define the Gate Pins
#define GATE_PIN_01 10 // PC0
#define GATE_PIN_02 11 // PC1
#define GATE_PIN_03 12 // PC2
#define GATE_PIN_04 13 // PC3
#define GATE_PIN_05 14 // PA1
#define GATE_PIN_06 15 // PA2
#define GATE_PIN_07 16 // PA3
#define GATE_PIN_08 0 // PA4

// Define IMIDI Pins
#define IMIDI_PIN_01 1 // PA5


SoftwareSerial serialCh1 =  SoftwareSerial(-1, IMIDI_PIN_01);
MIDI_CREATE_INSTANCE(SoftwareSerial, serialCh1, IMIDI_CH1);

int getGatePin(int id) {
  switch (id) {
    case 1:
      return GATE_PIN_01;
    case 2:
      return GATE_PIN_02;
    case 3:
      return GATE_PIN_03;
    case 4:
      return GATE_PIN_04;
    case 5:
      return GATE_PIN_05;
    case 6:
      return GATE_PIN_06;
    case 7:
      return GATE_PIN_07;
    case 8:
      return GATE_PIN_08;
    default:
      return -1;
  }
}

void setupGates() {
  for (int x = 1; x <= 8; x = x + 1) {
    int gatePin = getGatePin(x);
    pinMode(gatePin, OUTPUT);
    digitalWrite(gatePin, LOW);
  }
}

void handleNoteOn(byte channel, byte note, byte velocity) {
  int gatePin = getGatePin(channel);
  if (gatePin != -1) {
    digitalWrite(gatePin, HIGH);
  }
  
  IMIDI_CH1.sendNoteOn(note, velocity, 1);
}

// Callback for Note Off messages
void handleNoteOff(byte channel, byte note, byte velocity) {
  int gatePin = getGatePin(channel);
  if (gatePin != -1) {
    digitalWrite(gatePin, LOW);
  }

  IMIDI_CH1.sendNoteOff(note, velocity, 1);
}

void setup() {
  setupGates();

  Serial.begin(31250);
   // Initialize the MIDI library
  MIDI.begin(MIDI_CHANNEL_OMNI); // Listen to all MIDI channels
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);

  Serial.println("MIDI Listening Started!");

  pinMode(IMIDI_PIN_01, OUTPUT);
  serialCh1.begin(31250);
  IMIDI_CH1.begin();
}

void loop() {
  MIDI.read();
}