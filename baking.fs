#version 330

out vec4 finalColor;

// Uniforms for the dissonance calculation
uniform int numVoices;
uniform int numPartials;
uniform float otherVoicesDissonance;
uniform float voiceFreqs[32];
uniform float voiceAmplitudes[32];
uniform vec4 viewInts; // We get screen width/height from this
uniform float maxHeight;

// The size of the surface in world coordinates
const float SURFACE_WIDTH = 4.0;
const float SURFACE_HEIGHT = 4.0;

/* ============================================================================ */
/*                  Dissonance Calculation Functions                            */
/* ============================================================================ */

float pairwiseDissonance(float f1, float a1, float f2, float a2) {
    if (a1 == 0.0 || a2 == 0.0) return 0.0;
    float s1 = 3.5;
    float s2 = 5.75;
    float f_min = min(f1, f2);
    float f_max = max(f1, f2);
    float cbw = 25.0 + 75.0 * pow(1.0 + 1.4 * pow(f_min / 1000.0, 2.0), 0.69);
    if (cbw == 0.0) return 0.0;
    float f_diff_norm = (f_max - f_min) / cbw;
    return min(a1, a2) * (exp(-s1 * f_diff_norm) - exp(-s2 * f_diff_norm));
}

float getDissonanceAt(float x, float z) {
    float totalDissonance = 0.0;
    float coeff1;
    float coeff2;

    for (int i = 0; i < 2 * numPartials; i++) {
      if (i / numPartials == 0) {coeff1 = x;}
      else if (i / numPartials == 1) {coeff1 = z;}
      else {coeff1 = 1.0;}
      for (int j = i + 1; j < numVoices * numPartials;j++) {
        if (j / numPartials == 0) {coeff2 = x;}
        else if (j / numPartials == 1) {coeff2 = z;}
        else {coeff2 = 1.0;}
        totalDissonance += pairwiseDissonance(
            voiceFreqs[i] * coeff1, voiceAmplitudes[i],
            voiceFreqs[j] * coeff2, voiceAmplitudes[j]
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
    finalColor = vec4(height / maxHeight, 0.0, 0.0, 1.0);
}
