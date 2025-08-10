#version 330

//in vec2 fragCoord;
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
uniform vec4 viewInts;

const float scaling_factor = 1.0;

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

    for (int i = 0; i < 2; i++) {
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
    return (totalDissonance + otherVoicesDissonance) * scaling_factor;
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
    vec2 fragCoord = gl_FragCoord.xy;
    // Flip Y-coordinate
    fragCoord.y = viewInts.w - fragCoord.y;
    vec2 ndc = fragCoord / vec2(viewInts.z, viewInts.w) * 2.0 - 1.0;
    vec4 ray_eye = invProj * vec4(ndc.x, ndc.y, -1.0, 1.0);
    vec3 rayDir = normalize((invView * vec4(ray_eye.xyz, 0.0)).xyz);
    vec3 rayOrigin = cameraPos;

    // Bounding sphere pre-check
    vec3 sphereCenter = vec3(2.0, 0.5, 2.0);
    float sphereRadius = 3 * scaling_factor;
    vec3 oc = rayOrigin - sphereCenter;
    float b = dot(oc, rayDir);
    float c = dot(oc, oc) - sphereRadius*sphereRadius;
    float discriminant = b*b - c;

    if (discriminant < 0.0) {
        finalColor = vec4(0.05, 0.05, 0.1, 1.0);
    } else {
        /* --- Raymarching Loop --- */
        float t = -b - sqrt(discriminant);
        t = max(0.0, t); // Start at the sphere entry point, but not behind the camera
        // float t = rayOrigin.y * 0.75;
        // float t = 0.0;

        vec3 p = rayOrigin + t * rayDir;
        float h_prev = p.y - getDissonanceAt(p.x, p.z);
        bool hit = false;
        float min_h = h_prev;

        for (int i = 0; i < 32; i++) {
            float step = max(0.01, abs(h_prev) * 0.85) * (1.0 + t * 0.015);
            // float step = max(0.05, abs(h_prev) * 0.3);
            float t_curr = t + step;
            p = rayOrigin + t_curr * rayDir;
            float h_curr = p.y - getDissonanceAt(p.x, p.z);
            if (p.x < 0.0 || p.z < 0.0)
              break;

            min_h = min(min_h, h_curr);
            if (h_curr > min_h + 5.0) break; // Divergence check

            if (h_curr * h_prev < 0.0) {
                // Refinement phase: Adaptive binary search
                float t_lower = t;
                float t_upper = t_curr;
                int refinement_steps = int(max(2.0, 5.0 - t * 0.1));
                // int refinement_steps = 3;
                for (int j = 0; j < refinement_steps; j++) {
                    float t_mid = (t_lower + t_upper) / 2.0;
                    vec3 p_mid = rayOrigin + t_mid * rayDir;
                    float h_mid = p_mid.y - getDissonanceAt(p_mid.x, p_mid.z);
                    if (h_mid * h_prev < 0.0) {
                        t_upper = t_mid;
                    } else {
                        t_lower = t_mid;
                    }
                }
                t = (t_lower + t_upper) / 2.0;
                p = rayOrigin + t * rayDir;
                hit = true;
                break;
            }

            t = t_curr;
            h_prev = h_curr;
            if (t > 200.0) break;
        }

        /* --- Coloring and Lighting --- */
        if (hit) {
            vec3 normal = getNormal(p.x, p.z);
            vec3 lightDir = normalize(vec3(5, 100, 5));
            float diffuse = max(0.0, dot(normal, lightDir));
            vec3 color1 = vec3(0.1, 0.2, 0.8);
            vec3 color2 = vec3(1.0, 0.3, 0.2);
            vec3 surfaceColor = mix(color1, color2, clamp(p.y/scaling_factor * 1.5, 0.0, 1.0));
            float fog = 1.0 - clamp(t / 200.0, 0.0, 1.0);
            vec3 finalRgb = surfaceColor * (diffuse * 0.8 + 0.2);
            finalColor = vec4(mix(vec3(0.0), finalRgb, fog), 1.0);
            // finalColor = vec4(mix(vec3(0.0), surfaceColor, fog), 1.0);
        } else {
            finalColor = vec4(0.05, 0.05, 0.1, 1.0);
       }
    }
}
