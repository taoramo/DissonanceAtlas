#version 410

//in vec2 fragCoord;
out vec4 finalColor;

/* ============================================================================ */
/*                            CORE DEFINITIONS                                  */
/* ============================================================================ */

uniform mat4 invView;
uniform mat4 invProj;
uniform vec3 cameraPos;
uniform sampler2D heightmap;
uniform vec2 surfaceDimensions;
uniform vec4 viewInts;
uniform float maxHeight;

/* ============================================================================ */
/*                  Optimized Surface/Normal Functions                          */
/* ============================================================================ */

float getDissonanceAt(float x, float z) {
    vec2 uv = vec2(x / surfaceDimensions.x, z / surfaceDimensions.y);
    uv = clamp(uv, 0.0, 1.0);
    return texture(heightmap, uv).r * maxHeight;
}

vec3 getNormal(float x, float z) {
    vec2 e = vec2(0.01, 0.0);
    float h = getDissonanceAt(x, z);
    vec3 n = vec3(
        h - getDissonanceAt(x - e.x, z),
        e.x,
        h - getDissonanceAt(x, z - e.y)
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
    float sphereRadius = 3.0 * maxHeight;
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

        vec3 p = rayOrigin + t * rayDir;
        float h_prev = p.y - getDissonanceAt(p.x, p.z);
        bool hit = false;
        float min_h = h_prev;

        for (int i = 0; i < 64; i++) {
            float step = max(0.005, abs(h_prev) * 0.5) * (0.5 + t * 0.015);
            float t_curr = t + step;
            p = rayOrigin + t_curr * rayDir;
            float h_curr = p.y - getDissonanceAt(p.x, p.z);
            if (p.x > 4.0 || p.x < 0.0 || p.z > 4.0 || p.z < 0.0)
              break;

            min_h = min(min_h, h_curr);
            if (h_curr > min_h + 0.5) break; // Divergence check

            if (h_curr * h_prev < 0.0) {
                // Refinement phase: Adaptive binary search
                float t_lower = t;
                float t_upper = t_curr;
                int refinement_steps = int(max(2.0, 10.0 - t * 0.1));
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
            vec3 lightDir = normalize(vec3(0.5, 0.8, -0.5));
            float diffuse = max(0.0, dot(normal, lightDir));
            vec3 color1 = vec3(0.1, 0.2, 0.8);
            vec3 color2 = vec3(1.0, 0.3, 0.2);
            vec3 surfaceColor = mix(color1, color2, clamp(p.y, 0.0, 1.0));
            float fog = 1.0 - clamp(t / 200.0, 0.0, 1.0);
            vec3 finalRgb = surfaceColor * (diffuse * 0.8 + 0.2);
            finalColor = vec4(mix(vec3(0.0), finalRgb, fog), 1.0);
            // finalColor = vec4(mix(vec3(0.0), surfaceColor, fog), 1.0);
        } else {
            finalColor = vec4(0.05, 0.05, 0.1, 1.0);
       }
    }
}
