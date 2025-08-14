#ifndef DISSONANCE_H
#define DISSONANCE_H

typedef struct {
    float freq;
    float amp;
} Partial;

typedef struct {
    Partial partials[32];
    int numPartials;
} Voice;

void GenerateHarmonicSeries(Voice* voice, float baseFreq, float baseAmp, int numPartials, int max_partials);
float getCriticalBandwidth(float f);
float pairwiseDissonance(float f1, float a1, float f2, float a2);
float getDissonanceAtxz(float x, float z, Voice* voices, int numVoices);
float getDissonanceAtFreqs(Voice* voices, int numVoices);

#endif
