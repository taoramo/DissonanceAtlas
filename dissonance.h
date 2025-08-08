#ifndef DISSONANCE_H
#define DISSONANCE_H
#endif
#include <math.h>
#include "raymath.h"

/* Data structure for a single partial (frequency/amplitude pair) */
typedef struct {
    float freq;
    float amp;
} Partial;

/* Data structure for a voice, containing its partials */
typedef struct {
    Partial partials[32];
    int numPartials;
} Voice;

#ifndef DISSONANCE_IMPLEMENTATION

void GenerateHarmonicSeries(Voice* voice, float baseFreq, float baseAmp, int numPartials, int max_partials);
float getCriticalBandwidth(float f);
float pairwiseDissonance(float f1, float a1, float f2, float a2);
float getDissonanceAtxz(float x, float z, Voice* voices, int numVoices);
float getDissonanceAtFreqs(Voice* voices, int numVoices);

#else

/* Helper function to generate a standard harmonic series for a voice */
void GenerateHarmonicSeries(Voice* voice, float baseFreq, float baseAmp, int numPartials, int max_partials) {
    voice->numPartials = numPartials > max_partials ? max_partials : numPartials;
    for (int i = 0; i < voice->numPartials; i++) {
        voice->partials[i].freq = baseFreq * (i + 1);
        voice->partials[i].amp = baseAmp / (float)(i + 1);
    }
}

float pairwiseDissonance(float f1, float a1, float f2, float a2) {
    if (a1 == 0.0f || a2 == 0.0f) return 0.0f;
    float s1 = 3.5f;
    float s2 = 5.75f;
    float f_min = fminf(f1, f2);
    float f_max = fmaxf(f1, f2);
    float cbw = 25.0f + 75.0f * powf(1.0f + 1.4f * powf(f_min / 1000.0f, 2.0f), 0.69f);
    if (cbw == 0.0f) return 0.0f;
    float f_diff_norm = (f_max - f_min) / cbw;
    return fminf(a1, a2) * (expf(-s1 * f_diff_norm) - expf(-s2 * f_diff_norm));
}

/* Main dissonance calculation for a set of voices */
float getxzDissonance(float x, float z, float *voiceFreqs, float *voiceAmplitudes, int numVoices, int numPartials) {
    float totalDissonance = 0.0f;
    float coeff1;
    float coeff2;

    for (int i = 0; i < 3; i++) {
      if (i / numVoices == 1) {coeff1 = x;} 
      else if (i / numVoices == 2) {coeff1 = z;}
      else {coeff1 = 1.0;}
      for (int j = i + 1; j < numVoices * numPartials;j++) {
        if (j / numVoices == 1) {coeff1 = x;} 
        else if (j / numVoices == 2) {coeff1 = z;}
        else {coeff2 = 1.0;}
        totalDissonance += pairwiseDissonance(
            voiceFreqs[i] * coeff1, voiceAmplitudes[i],
            voiceFreqs[j] * coeff2, voiceAmplitudes[j]
      );
      }
    }
    return totalDissonance;
}
#endif
