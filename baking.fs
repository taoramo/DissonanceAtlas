#version 330

out vec4 finalColor;

// Uniforms for the dissonance calculation
uniform int numVoices;
uniform int numPartials;
uniform float otherVoicesDissonance;
uniform float voiceFreqs[256];
uniform float voiceAmplitudes[256];
uniform vec4 viewInts; // We get screen width/height from this
uniform float maxHeight;

// The size of the surface in world coordinates
const float SURFACE_WIDTH = 4.0;
const float SURFACE_HEIGHT = 4.0;

#define PLOMP_A 3.5
#define PLOMP_B 5.75

/* ============================================================================ */
/*                  Dissonance Calculation Functions                            */
/* ============================================================================ */

float pairwiseDissonance(float f1, float f2, float a1, float a2) {
    if (a1 == 0.0 || a2 == 0.0) return 0.0;
    float fmini = min(f1, f2);
    float freq_diff = abs(f1 - f2);
    float amp_prod = a1 * a2;
    float s = 0.024/(0.021 * fmini + 19);
    return (amp_prod * (exp(-PLOMP_A * s * freq_diff) - exp(-PLOMP_B * s * freq_diff)));
}

float getDissonanceAt(float x, float z) {
    float totalDissonance = 0.0;
    float coeff1;
    float coeff2;

    for (int i = 0; i < 2; i++) {
      if (i / numPartials == 0) {coeff1 = x;}
      else if (i / numPartials == 1) {coeff1 = z;}
      else {coeff1 = 1.0;}
      for (int j = i + 1; j < numVoices * numPartials;j++) {
        if (j / numPartials == 0) {coeff2 = x;}
        else if (j / numPartials == 1) {coeff2 = z;}
        else {coeff2 = 1.0;}
        totalDissonance += pairwiseDissonance(
            voiceFreqs[i] * coeff1, voiceFreqs[j] * coeff2,
            voiceAmplitudes[i], voiceAmplitudes[j]
      );
      }
    }
    return totalDissonance + otherVoicesDissonance;
}

/* ============================================================================ */
/*                                  MAIN                                        */
/* ============================================================================ */
void main() {
    // Convert fragment coordinate to world coordinate (x, z)
    vec2 worldCoord = gl_FragCoord.xy / vec2(viewInts.z, viewInts.w) * vec2(SURFACE_WIDTH, SURFACE_HEIGHT);

    // Calculate the dissonance (height) at this point
    float height = getDissonanceAt(worldCoord.x, worldCoord.y);

    // Output the normalized height to the red channel.
    finalColor = vec4(height / (maxHeight * 10), 0.0, 0.0, 1.0);
}
