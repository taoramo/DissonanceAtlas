#define PLATFORM_MACOS
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#define DISSONANCE_IMPLEMENTATION
#include "dissonance.h"
#include "vulkan_backend.h"

#define GLSL_VERSION            330

#include <vulkan/vulkan_metal.h>


/* ============================================================================ */
/*                            CORE DEFINITIONS                                  */
/* ============================================================================ */
#define MAX_PARTIALS 6
#define NUM_VOICES 3


/* ============================================================================ */
/*                                  MAIN                                        */
/* ============================================================================ */
int main(void) {
    /* --- Initialization --- */
    const int screenWidth = 1280;
    const int screenHeight = 720;
    // SetConfigFlags(FLAG_MSAA_4X_HINT); // We'll handle MSAA in Vulkan
    InitWindow(screenWidth, screenHeight, "Dissonance Atlas Vulkan");
    SetTargetFPS(60);

    // Initialize Vulkan backend
    vulkan_init(GetWindowHandle());

    /* --- Camera Setup --- */
    Camera camera = { 0 };
    camera.target = (Vector3){ 1.5f, 1.0f, 1.5f };
    camera.up = (Vector3){ 0.0f, -1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position = (Vector3){ 4.0f, 5.0f, 1.0f };

    /* --- Voice Data Setup --- */
    int numVoices = NUM_VOICES;
    int numPartials = MAX_PARTIALS;
    Voice voices[NUM_VOICES] = { 0 };
    float base_freq = 220.0f;

    GenerateHarmonicSeries(&voices[0], base_freq, 1.0f, 8, MAX_PARTIALS);
    GenerateHarmonicSeries(&voices[1], base_freq, 1.0f, 8, MAX_PARTIALS);
    GenerateHarmonicSeries(&voices[2], base_freq, 1.0f, 8, MAX_PARTIALS);

    float voiceFreqs[NUM_VOICES * MAX_PARTIALS];
    float voiceAmplitudes[NUM_VOICES * MAX_PARTIALS];

    int totalPartials = 0;
    for (int i = 0; i < numVoices; i++) {
      for (int j = 0; j < numPartials; j++) {
        voiceFreqs[totalPartials] = voices[i].partials[j].freq;
        voiceAmplitudes[totalPartials] = voices[i].partials[j].amp;
        totalPartials++;
      }
    }

    float otherVoicesDissonance = 0;
    for (int i = 2 * numPartials; i < numVoices * numPartials; i++) {
        for (int j = i + 1; j < numVoices * numPartials; j++) {
          otherVoicesDissonance += pairwiseDissonance(voiceFreqs[i], voiceAmplitudes[i], voiceFreqs[j], voiceAmplitudes[j]);
        }
    }

   // --- Main Game Loop ---
    while (!WindowShouldClose()) {
        // --- Update ---
        UpdateCameraPro(
            &camera,
            (Vector3){
                (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) * 0.1f - (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * 0.1f,
                (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) * 0.1f - (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * 0.1f,
                0.0f
            },
            (Vector3){ GetMouseDelta().x * 0.05f, GetMouseDelta().y * 0.05f, 0.0f },
            GetMouseWheelMove() * 2.0f
        );

        // --- Prepare Uniforms for Vulkan ---
        Uniforms uniforms = { 0 };
        Matrix view = GetCameraMatrix(camera);
        Matrix proj = MatrixPerspective(camera.fovy * DEG2RAD, (double)screenWidth / screenHeight, 0.1, 1000.0);
        uniforms.invView = MatrixInvert(view);
        uniforms.invProj = MatrixInvert(proj);
        uniforms.cameraPos = camera.position;
        uniforms.numVoices = numVoices;
        uniforms.numPartials = numPartials;
        uniforms.otherVoicesDissonance = otherVoicesDissonance;
        for(int i=0; i<NUM_VOICES * MAX_PARTIALS; ++i) {
            uniforms.voiceFreqs[i] = voiceFreqs[i];
            uniforms.voiceAmplitudes[i] = voiceAmplitudes[i];
        }
        uniforms.viewInts = (Vector4){ 0.0, 0.0, (float)screenWidth, (float)screenHeight };

        // --- Draw with Vulkan ---
        vulkan_draw_frame(&uniforms);

        // --- Raylib overlay for text ---
        BeginDrawing();
            DrawText("Dissonance Surface (Vulkan)", 10, 10, 20, SKYBLUE);
            DrawFPS(screenWidth - 90, 10);
        EndDrawing();
    }

    // --- Cleanup ---
    vulkan_cleanup();
    CloseWindow();
    return 0;
}
