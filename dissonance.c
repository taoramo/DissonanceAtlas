#include "dissonance.h"
#include <math.h>

#define PLOMP_A 3.5f
#define PLOMP_B 5.75f

void generate_harmonic_series(Voices* voices, float baseFreq, float baseAmps, int numPartials) {
  if (voices->count + 1 > MAX_VOICES) return;
  int partial_count = voices->count * MAX_PARTIALS;
  for (int i = 0; i < numPartials; i++)
  {
    voices->freqs[partial_count + i] = baseFreq * i;
    voices->amps[partial_count + i] = baseAmps / i;
  }
  for (int i = 0; i < MAX_PARTIALS - numPartials; i++)
    voices->amps[partial_count + numPartials + i] = 0.0f;
  voices->count++;
}

float pairwise_dissonance(float f1, float f2, float a1, float a2) {
    float freq_diff = fabsf(f1 - f2);
    float amp_prod = a1 * a2;
    return (amp_prod * (expf(-PLOMP_A * freq_diff) - expf(-PLOMP_B * freq_diff)));
}

//by convention, keep x and z voices as indeces 0 and 1
float get_xz_dissonance(Voices *voices, float coeff_x, float coeff_z, float otherVoicesDissonance) {
  float xz_dissonance = 0;
  for (int i = 0; i < MAX_PARTIALS; i++)
    voices->freqs[i] *= coeff_x; 
  for (int i = MAX_PARTIALS; i < 2 * MAX_PARTIALS; i++)
    voices->freqs[i] *= coeff_z; 
  for (int i = 0; i < 2; i++)
    for (int j = i + 1; j < voices->count; j++)
      xz_dissonance += pairwise_dissonance(voices->freqs[i], voices->freqs[j], voices->amps[i], voices->amps[j]);
  return otherVoicesDissonance + xz_dissonance;
}

float calculate_dissonance(Voices* voices) {
  float total_dissonance = 0.0f;
  for (int i = 0; i < voices->count; ++i) {
    float f1 = voices->freqs[i];
    float amp1 = voices->amps[i];
    for (int j = i + 1; j < voices->count; ++j) {
      float freq_diff = fabsf(f1 - voices->freqs[j]);
      float amp_prod = amp1 * voices->amps[j];
      total_dissonance += amp_prod * (expf(-PLOMP_A * freq_diff) - expf(-PLOMP_B * freq_diff));
    }
  }
  return total_dissonance;
}
