#version 330

in vec2 fragCoord;
out vec4 finalColor;

/* ============================================================================ */
/*                            CORE DEFINITIONS                                  */
/* ============================================================================ */

uniform mat4 invView;
uniform mat4 invProj;
uniform vec3 cameraPos;
uniform int numVoices;
uniform int numPartials;
uniform float otherVoicesDissonance;
uniform float voiceFreqs[32];
uniform float voiceAmplitudes[32];

/* ============================================================================ */
/*                  GLSL REPLICATION OF MATH (UPDATED)                          */
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

    for (int i = 0; i < 3; i++) {
      if (i / numPartials == 1) {coeff1 = x;} 
      else if (i / numPartials == 2) {coeff1 = z;}
      else {coeff1 = 1.0;}
      for (int j = i + 1; j < numVoices * numPartials;j++) {
        if (j / numPartials == 1) {coeff1 = x;} 
        else if (j / numPartials == 2) {coeff1 = z;}
        else {coeff2 = 1.0;}
        totalDissonance += pairwiseDissonance(
            voiceFreqs[i] * coeff1, voiceAmplitudes[i],
            voiceFreqs[j] * coeff2, voiceAmplitudes[j]
      );
      }
    }
    return totalDissonance + otherVoicesDissonance;
}

vec3 getNormal(float x, float z) {
    vec2 e = vec2(0.01, 0.0);
    float h = getDissonanceAt(x, z);
    vec3 n = vec3(
        h - getDissonanceAt(x - e.x, z - e.y),
        e.x,
        h - getDissonanceAt(x - e.y, z - e.x)
    );
    return normalize(n);
}

/* ============================================================================ */
/*                                  MAIN                                        */
/* ============================================================================ */
void main() {
    /* --- Ray Generation --- */
    vec2 ndc = fragCoord * 2.0 - 1.0;
    vec4 ray_eye = invProj * vec4(ndc.x, ndc.y, -1.0, 1.0);
    vec3 rayDir = normalize((invView * vec4(ray_eye.xyz, 0.0)).xyz);
    vec3 rayOrigin = cameraPos;

    /* --- Raymarching Loop --- */
    float t = 0.0;
    vec3 p = rayOrigin;
    float h_prev = p.y - getDissonanceAt(p.x, p.z);
    bool hit = false;

    for (int i = 0; i < 128; i++) {
        t += max(0.01, h_prev * 0.5);
        p = rayOrigin + t * rayDir;
        float h_curr = p.y - getDissonanceAt(p.x, p.z);
        if (h_curr * h_prev < 0.0) {
            hit = true;
            break;
        }
        h_prev = h_curr;
        if (t > 200.0) break;
    }

    /* --- Coloring and Lighting --- */
    if (hit) {
    //      /* No normal or lighting calculations are needed. */
    //
    //     /* Color is determined directly by the surface height (dissonance). */
    //     vec3 color1 = vec3(0.1, 0.2, 0.8); // Blue for low dissonance
    //     vec3 color2 = vec3(1.0, 0.3, 0.2); // Red for high dissonance
    //
    //     /* The y-coordinate of the intersection point 'p' is the dissonance value. */
    //     vec3 surfaceColor = mix(color1, color2, clamp(p.y / 10.0, 0.0, 1.0));
    //
    //     // /* Apply fog for depth perception. */
    //     // float fog = 1.0 - clamp(t / 200.0, 0.0, 1.0);
    //     // finalColor = vec4(mix(vec3(0.0), surfaceColor, fog), 1.0);
    //
    // } else {
    //     /* Background color if no hit (Unchanged) */
    //     finalColor = vec4(0.05, 0.05, 0.1, 1.0);
    // }
        // vec3 normal = getNormal(p.x, p.z);
        // vec3 lightDir = normalize(vec3(0.5, 0.8, -0.5));
        // float diffuse = max(0.0, dot(normal, lightDir));
        vec3 color1 = vec3(0.1, 0.2, 0.8);
        vec3 color2 = vec3(1.0, 0.3, 0.2);
        vec3 surfaceColor = mix(color1, color2, clamp(p.y, 0.0, 1.0));
        // vec3 finalRgb = surfaceColor * (diffuse * 0.8 + 0.2);
        float fog = 1.0 - clamp(t / 200.0, 0.0, 1.0);
        // finalColor = vec4(mix(vec3(0.0), finalRgb, fog), 1.0);
        finalColor = vec4(mix(vec3(0.0), surfaceColor, fog), 1.0);
        // finalColor = vec4(finalRgb, 1.0);
    } else {
        finalColor = vec4(0.05, 0.05, 0.1, 1.0);
   }
}
