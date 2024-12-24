#ifndef CUSTOM_OSCILLATOR_H
#define CUSTOM_OSCILLATOR_H

#include <stdint.h>

class CustomOscillator {
public:
    CustomOscillator(const int8_t* wavetable, uint16_t tableSize, uint32_t sampleRate)
        : wavetable(wavetable), tableSize(tableSize), sampleRate(sampleRate), phase(0), phaseIncrement(0) {}

    // Set the desired frequency
    void setFrequency(float frequency) {
        // Calculate the phase increment based on frequency, table size, and sample rate
        phaseIncrement = (uint32_t)((frequency * tableSize) / sampleRate);
    }

    // Generate the next sample
    int8_t next() {
        // Calculate the current index in the wavetable
        uint16_t index = (uint16_t)phase & (tableSize - 1); // Use upper 8 bits of 32-bit phase accumulator
        // Increment the phase accumulator
        phase += phaseIncrement;

        // Return the wavetable value at the current index
        return wavetable[index];
    }

public:
    const int8_t* wavetable;  // Pointer to the wavetable
    uint16_t tableSize;        // Size of the wavetable
    uint32_t sampleRate;       // Sample rate in Hz
    uint32_t phase;            // Phase accumulator
    uint32_t phaseIncrement;   // Phase increment based on frequency
};

#endif
