#include <Arduino.h>
#include <SoftwareSerial.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "utils.h"
#include "SimpleMIDI.h"
#include "CustomOscillator.h"
#include "ModeHandler.h"
#include "samples/ride.h"
#include "samples/snare.h"
#include "samples/perc.h"
#include "samples/clap.h"

#define GATE_PIN PIN_PA7
#define LOGGER_PIN_TX PIN_PB4
#define PIN_CV1 PIN_PA1
#define PIN_CV2 PIN_PA2
#define TOGGLE_PIN PIN_PC2
#define TOGGLE_LED PIN_PC3

SimpleMIDI MIDI;
SoftwareSerial logger = SoftwareSerial(-1, LOGGER_PIN_TX);
ModeHandler modes = ModeHandler(TOGGLE_PIN, 3, 300);

void toggleLED() {
  delay(500);
  digitalWrite(TOGGLE_LED, HIGH);
  delay(500);
  digitalWrite(TOGGLE_LED, LOW);
}

void setupTimer() {
    TCB0.CTRLA |= TCB_ENABLE_bm; // counting value

    // Uses 20Mhz clock & divide it by for here
    // So, 1 tick is 0.1us
    TCB0.CTRLA |= TCB_CLKSEL_CLKDIV2_gc; // clock div by 1

    TCB0.CCMP = 10000000 / (RIDE_SAMPLE_RATE); // Set the compare value
  
    // Enabling inturrupts
    TCB0.CTRLB |= TCB_CNTMODE_INT_gc; // Timer interrupt mode (periodic interrupts)
    TCB0.INTCTRL = TCB_CAPT_bm; // Enable Capture interrupt
    sei();
}

const uint8_t* sampleData = 0;
int sampleLenght;
int index = 0;
// TCB0 Interrupt Service Routine
ISR(TCB0_INT_vect) {
  if (sampleData != 0 && index < sampleLenght) {
    // DAC0.DATA = SNARE_SAMPLE[index];
    // DAC0.DATA = RIDE_SAMPLE[index];
    DAC0.DATA = sampleData[index];
    // DAC0.DATA = CLAP_SAMPLE[index];
    index += 1;
  } else {
    DAC0.DATA = 127;
  }

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

  logger.println("MCO 0.2.0 Started!");
}

void pickSound(uint8_t note) {
  // Here we pick sounds for C, D, E, F (regardless of their octave)
  byte soundIndex = note % 12;
  switch (soundIndex)
  {
    case 0:
      sampleData = SNARE_SAMPLE;
      sampleLenght = SNARE_SAMPLE_LENGTH;
      index = 0;
      break;

    case 2:
      sampleData = CLAP_SAMPLE;
      sampleLenght = CLAP_SAMPLE_LENGTH;
      index = 0;
      break;

    case 4:
      sampleData = PERC_SAMPLE;
      sampleLenght = PERC_SAMPLE_LENGTH;
      index = 0;
      break;

    case 5:
      sampleData = RIDE_SAMPLE;
      sampleLenght = RIDE_SAMPLE_LENGTH;
      index = 0;
      break;
    
    default:
      sampleData = 0;
      break;
  }
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
        digitalWrite(GATE_PIN, HIGH);
        pickSound(data1);
      } else if (type == MIDI_NOTE_OFF) {
        digitalWrite(GATE_PIN, LOW);
      } else if (type == MIDI_CONTROL_CHANGE) {
          
      } else {
          logger.print("Unknown MIDI Data| ");
      }
  }

  // mode related code
  if (modes.update()) {
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
