#ifndef DISSONANCE_H
#define DISSONANCE_H

#include <stdlib.h>
#include "raylib.h"
#define MAX_PARTIALS 32
#define MAX_VOICES 8 

typedef struct {
    int count;
    int numPartials[MAX_VOICES];
    float baseFreq;
    float baseAmp;
    float freqs[MAX_VOICES * MAX_PARTIALS];
    float amps[MAX_VOICES * MAX_PARTIALS];
} Voices;

void generate_harmonic_series(Voices* voice, float baseFreq, float baseAmp, int numPartials);
float pairwise_dissonance(float f1, float a1, float f2, float a2);
float get_xz_dissonance(Voices *voices, float coeff_x, float coeff_z, float otherVoicesDissonance);
float calculate_dissonance(Voices* voices);

void handle_input(Camera3D *cameraMesh, Voices *voices, float otherVoicesDissonance);
#endif
