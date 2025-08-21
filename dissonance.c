#include "dissonance.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define PLOMP_A 3.5f
#define PLOMP_B 5.75f

void generate_harmonic_series(Voices *voices, float baseFreq, float baseAmps, int numPartials) {
  if (voices->count + 1 > MAX_VOICES)
    return;
  int partial_count = voices->count * MAX_PARTIALS;
  for (int i = 0; i < numPartials; i++) {
    voices->freqs[partial_count + i] = baseFreq * (i + 1);
    voices->amps[partial_count + i] = baseAmps / (i + 1);
  }
  for (int i = 0; i < MAX_PARTIALS - numPartials; i++)
    voices->amps[partial_count + numPartials + i] = 0.0f;
  voices->count++;
}

float pairwise_dissonance(float f1, float a1, float f2, float a2) {
  if (a1 == 0.0 || a2 == 0.0)
    return 0.0;
  float s1 = 3.5;
  float s2 = 5.75;
  float f_min = fmin(f1, f2);
  float f_max = fmax(f1, f2);
  float cbw = 25.0 + 75.0 * powf(1.0 + 1.4 * powf(f_min / 1000.0, 2.0), 0.69);
  if (cbw == 0.0)
    return 0.0;
  float f_diff_norm = (f_max - f_min) / cbw;
  return fmin(a1, a2) * (expf(-s1 * f_diff_norm) - expf(-s2 * f_diff_norm));
}

// by convention, keep x and z voices as indeces 0 and 1
float get_xz_dissonance(Voices *voices, float coeff_x, float coeff_z, float otherVoicesDissonance) {
  float xz_dissonance = 0;

  float local_freqs[MAX_VOICES * MAX_PARTIALS];
  memcpy(local_freqs, voices->freqs, sizeof(local_freqs));

  for (int i = 0; i < MAX_PARTIALS; i++)
    local_freqs[i] *= coeff_x;
  for (int i = MAX_PARTIALS; i < 2 * MAX_PARTIALS; i++)
    local_freqs[i] *= coeff_z;

  for (int i = 0; i < 2; i++)
    for (int j = i + 1; j < voices->count; j++)
      xz_dissonance += pairwise_dissonance(local_freqs[i], voices->amps[i], local_freqs[j], voices->amps[j]);

  return otherVoicesDissonance + xz_dissonance;
}

float calculate_dissonance(Voices *voices, int starting_index) {
  float total_dissonance = 0.0f;
  // for (int i = 0; i < voices->count * MAX_PARTIALS; i++) {
  //   printf("freq: %f, amp: %f, i: %d\n", voices->freqs[i], voices->amps[i], i);
  // }
  for (int i = starting_index; i < voices->count * MAX_PARTIALS; ++i) {
    float f1 = voices->freqs[i];
    float amp1 = voices->amps[i];
    for (int j = i + 1; j < voices->count * MAX_PARTIALS; ++j) {
      float fmin = fminf(f1, voices->freqs[j]);
      float fmax = fmaxf(f1, voices->freqs[j]);
      float amp_prod = amp1 * voices->amps[j];
      float cbw = 25.0 + 75.0 * powf(1.0 + 1.4 * powf(fmin / 1000.0, 2.0), 0.69);
      float f_diff_norm = (fmax - fmin) / cbw;
      total_dissonance += amp_prod * (expf(-PLOMP_A * f_diff_norm) - expf(-PLOMP_B * f_diff_norm));
      // printf("total_dissonance: %f\n", total_dissonance);
    }
  }
  return total_dissonance;
}
