#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "tones.h"

void setup() {
    // Set PA2 as input
    PORTA.DIRCLR = PIN7_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;

    // Setup tones
    setupTone1();
    setupTone2();

    // Setup Serial Comm
    Serial0.begin(31250);

    setTone1Frequency(110);
    setTone2Frequency(120);
}

void loop() {
    // if(PORTA.IN & PIN7_bm) {
    //   // When high
    //   setTone2Frequency(220);
    // } else {
    //   // When low
    //   setTone2Frequency(110);
    // }
}
