#include <Arduino.h>
#include <SoftwareSerial.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "utils.h"
#include "SimpleMIDI.h"
#include "CustomOscillator.h"
#include "ModeHandler.h"
#include "tables/triangle_2048.h"
#include "tables/sin_2048.h"
#include "tables/square_2048.h"

#define GATE_PIN PIN_PA7
#define LOGGER_PIN_TX PIN_PB4
#define PIN_CV1 PIN_PA1
#define PIN_CV2 PIN_PA2
#define TOGGLE_PIN PIN_PC2
#define TOGGLE_LED PIN_PC3

SimpleMIDI MIDI;
SoftwareSerial logger = SoftwareSerial(-1, LOGGER_PIN_TX);
ModeHandler modes = ModeHandler(TOGGLE_PIN, 3, 300);

CustomOscillator osc1(TRIANGLE_2048_DATA, 2048, 10000);
CustomOscillator osc2(TRIANGLE_2048_DATA, 2048, 10000);
CustomOscillator osc3(TRIANGLE_2048_DATA, 2048, 10000);
CustomOscillator osc4(TRIANGLE_2048_DATA, 2048, 10000);
CustomOscillator osc5(TRIANGLE_2048_DATA, 2048, 10000);
CustomOscillator osc6(TRIANGLE_2048_DATA, 2048, 10000);
CustomOscillator osc7(TRIANGLE_2048_DATA, 2048, 10000);

uint8_t unisonCount = 1;

void updateWaveTables() {
  const int8_t* wavetable;
  int mode = modes.getMode();
  if (mode == 0) {
    wavetable = SIN_2048_DATA;
  } else if (mode == 1) {
    wavetable = TRIANGLE_2048_DATA;
  } else {
    wavetable = SQUARE_2048_DATA;
  }

  osc1.updateWaveTable(wavetable);
  osc2.updateWaveTable(wavetable);
  osc3.updateWaveTable(wavetable);
  osc4.updateWaveTable(wavetable);
  osc5.updateWaveTable(wavetable);
  osc6.updateWaveTable(wavetable);
  osc7.updateWaveTable(wavetable);
}

void toggleLED() {
  delay(500);
  digitalWrite(TOGGLE_LED, HIGH);
  delay(500);
  digitalWrite(TOGGLE_LED, LOW);
}

void updateNote(byte note) {
  float cv1 = analogRead(PIN_CV1) / 1023.0;
  float range = cv1 * 12;
  osc1.setFrequency(midiToFrequency(note));
  osc2.setFrequency(midiToFrequency(note - range));
  osc3.setFrequency(midiToFrequency(note - (range / 2.0)));
  osc4.setFrequency(midiToFrequency(note - (range / 4.0)));
  
  osc5.setFrequency(midiToFrequency(note + (range / 4.0)));
  osc6.setFrequency(midiToFrequency(note + (range / 2.0)));
  osc7.setFrequency(midiToFrequency(note + range));
  
  

  float cv2 = analogRead(PIN_CV2) / 1023.0;
  unisonCount = 1 + cv2 * 6;
}

uint8_t getUnison() {
    int values = 0; // Variable to hold the sum of oscillator values
    switch (unisonCount) {
        case 1:
            values = osc1.next();
            return values + 127;

        case 2:
            values = osc1.next() + osc2.next();
            return (values / 2) + 127;

        case 3:
            values = osc1.next() + osc2.next() + osc3.next();
            return (values / 3) + 127;

        case 4:
            values = osc1.next() + osc2.next() + osc3.next() + osc4.next();
            return (values / 4) + 127;

        case 5:
            values = osc1.next() + osc2.next() + osc3.next() + osc4.next() + osc5.next();
            return (values / 5) + 127;

        case 6:
            values = osc1.next() + osc2.next() + osc3.next() + osc4.next() + osc5.next() + osc6.next();
            return (values / 6) + 127;

        case 7:
            values = osc1.next() + osc2.next() + osc3.next() + osc4.next() + osc5.next() + osc6.next() + osc7.next();
            return (values / 7) + 127;

        default:
            return 127; // Default to midpoint if unisonCount is out of range
    }
}

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
  pinMode(PIN_CV1, INPUT);
  pinMode(PIN_CV2, INPUT);
 
  // DAC0 setup for sending velocity via the PA6 pin
  VREF.CTRLA |= VREF_DAC0REFSEL_4V34_gc; //this will force it to use VDD as the VREF
  VREF.CTRLB |= VREF_DAC0REFEN_bm;
  DAC0.CTRLA = DAC_ENABLE_bm | DAC_OUTEN_bm;
  DAC0.DATA = 0;

  // Setup MIDI
  MIDI.begin(31250);

  // Timer related
  setupTimer();
  
  // Modes
  modes.begin();
  pinMode(TOGGLE_LED, OUTPUT);
  updateWaveTables();

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
        updateNote(data1);
        // logger.printf("NOTE ON: %d, %d, %ld\n", data1, (int)freq, osc1.phaseIncrement);
        digitalWrite(GATE_PIN, HIGH);
      } else if (type == MIDI_NOTE_OFF) {
        digitalWrite(GATE_PIN, LOW);
      } else if (type == MIDI_CONTROL_CHANGE) {
          
      } else {
          // logger.print("Unknown MIDI Data| ");
      }
  }

  if (read) {
    // Output sine wave to DAC
    DAC0.DATA = getUnison();
    read = false;
  }

  // mode related code
  if (modes.update()) {
    updateWaveTables();

    byte currentMode = modes.getMode();
    if (currentMode == 0) {
      toggleLED();
    } else if (currentMode == 1) {
      toggleLED();
      toggleLED();
    } else if (currentMode == 2) {
      toggleLED();
      toggleLED();
      toggleLED();
    }
  }
}
