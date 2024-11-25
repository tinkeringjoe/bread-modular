#include <MIDI.h>

// Create a MIDI object
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

// Define the LED pin
const int GATE_PIN_01 = 11;
const int GATE_PIN_02 = 12;
const int GATE_PIN_03 = 13;
const int GATE_PIN_04 = 14;
const int GATE_PIN_05 = 15;
const int GATE_PIN_06 = 16;
const int GATE_PIN_07 = 17;
const int GATE_PIN_08 = 18;
const int GATE_PIN_09 = 33;
const int GATE_PIN_10 = 34;
const int GATE_PIN_11 = 35;
const int GATE_PIN_12 = 36;
const int GATE_PIN_13 = 37;
const int GATE_PIN_14 = 38;
const int GATE_PIN_15 = 39;
const int GATE_PIN_16 = 40;

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
    case 9:
      return GATE_PIN_09;
    case 10:
      return GATE_PIN_10;
    case 11:
      return GATE_PIN_11;
    case 12:
      return GATE_PIN_12;
    case 13:
      return GATE_PIN_13;
    case 14:
      return GATE_PIN_14;
    case 15:
      return GATE_PIN_15;
    case 16:
      return GATE_PIN_16;
    default:
      return -1;
  }
}

void setupGates() {
  for (int x = 1; x <= 16; x = x + 1) {
    int gatePin = getGatePin(x);
    pinMode(gatePin, OUTPUT);
    digitalWrite(gatePin, LOW);
  }
}

void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait for Serial Monitor to open
  }
  Serial.println("MIDI Listener Started!");
  Serial.print("CPU Frequency: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");

  // Set up the LED pin as output
  setupGates();

  // Initialize UART1 for MIDI communication
  // Parameters: baud rate, serial config, RX pin, TX pin
  Serial1.begin(31250, SERIAL_8N1, 42, -1); // RX on GPIO42, no TX

  // Initialize the MIDI library
  MIDI.begin(MIDI_CHANNEL_OMNI); // Listen to all MIDI channels
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
}

void loop() {
  // Check for MIDI messages
  MIDI.read();
}

// Callback for Note On messages
void handleNoteOn(byte channel, byte note, byte velocity) {
  Serial.print("Note On - Channel: ");
  Serial.print(channel);
  Serial.print(", Note: ");
  Serial.print(note);
  Serial.print(", Velocity: ");
  Serial.println(velocity);

  // Turn on the LED if Note On is received
  for (int x = 1; x <= 16; x = x + 1) {
    int gatePin = getGatePin(x);
    digitalWrite(gatePin, HIGH);
  }
}

// Callback for Note Off messages
void handleNoteOff(byte channel, byte note, byte velocity) {
  Serial.print("Note Off - Channel: ");
  Serial.print(channel);
  Serial.print(", Note: ");
  Serial.print(note);
  Serial.print(", Velocity: ");
  Serial.println(velocity);

  // Turn off the LED if Note Off is received
  for (int x = 1; x <= 16; x = x + 1) {
    int gatePin = getGatePin(x);
    digitalWrite(gatePin, LOW);
  }
}
