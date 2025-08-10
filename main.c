#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>
#define DISSONANCE_IMPLEMENTATION
#include "dissonance.h"


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
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "T3 Chat - Dissonance Raymarcher (Complex Tones)");
    SetTargetFPS(60);

    /* --- Camera Setup --- */
    Camera camera = { 0 };
    camera.target = (Vector3){ 1.5f, 1.0f, 1.5f };
    camera.up = (Vector3){ 0.0f, -1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position = (Vector3){ 4.0f, 5.0f, 1.0f };

    Camera2D camera2d = { 0 };
    camera2d.zoom = 1.0f;

    /* --- Shader and Uniforms --- */
    Shader shader = LoadShader(0, "dissonance.fs");
    int invViewLoc = GetShaderLocation(shader, "invView");
    int invProjLoc = GetShaderLocation(shader, "invProj");
    int cameraPosLoc = GetShaderLocation(shader, "cameraPos");
    int numVoicesLoc = GetShaderLocation(shader, "numVoices");
    int numPartialsLoc = GetShaderLocation(shader, "numPartials");
    int voiceFreqsLoc = GetShaderLocation(shader, "voiceFreqs");
    int voiceAmplitudesLoc = GetShaderLocation(shader, "voiceAmplitudes");
    int otherVoicesDissonanceLoc = GetShaderLocation(shader, "otherVoicesDissonance");
    int viewIntsLoc = GetShaderLocation(shader, "viewInts");

    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);

    /* --- Voice Data Setup --- */
    //numVoices is number of voices other than base, x, z
    int numVoices = NUM_VOICES;
    int numPartials = MAX_PARTIALS;
    Voice voices[NUM_VOICES] = { 0 };
    float base_freq = 220.0f;

    /* Voice 0: Dynamic X-axis */
    GenerateHarmonicSeries(&voices[0], base_freq, 1.0f, 8, MAX_PARTIALS);
    /* Voice 2: Dynamic Z-axis. */
    GenerateHarmonicSeries(&voices[1], base_freq, 1.0f, 8, MAX_PARTIALS);
    /* Voice 2: base_freq */
    GenerateHarmonicSeries(&voices[2], base_freq, 1.0f, 8, MAX_PARTIALS);

    /* Flatten voice data for the shader */
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
        // UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        UpdateCameraPro(
            &camera,
            (Vector3){
                IsKeyDown(KEY_W) * 0.1f - IsKeyDown(KEY_S) * 0.1f,
                IsKeyDown(KEY_D) * 0.1f - IsKeyDown(KEY_A) * 0.1f,
                IsKeyDown(KEY_R) * 0.1f - IsKeyDown(KEY_F) * 0.1f
            },
            (Vector3){
                IsKeyDown(KEY_RIGHT) * 0.1f - IsKeyDown(KEY_LEFT) * 0.1f, // Rotation: pitch
                IsKeyDown(KEY_UP) * 0.1f - IsKeyDown(KEY_DOWN) * 0.1f, // Rotation: yaw
                0.0f                       // Rotation: roll
            },
            GetMouseWheelMove() * 2.0f); // Move to target (zoom)

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            Ray ray = GetMouseRay(GetMousePosition(), camera);
            bool hit = false;
            Vector3 intersectionPoint = { 0 };
            float t = 0.0f;
            Vector3 p = ray.position;
            float h_prev = p.y - getxzDissonance(p.x, p.z, voiceFreqs, voiceAmplitudes, numVoices, numPartials) + otherVoicesDissonance;

            for (int i = 0; i < 256; i++) {
                t += fmaxf(0.01f, h_prev * 0.5f);
                p = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
                float h_curr = p.y - getxzDissonance(p.x, p.z, voiceFreqs, voiceAmplitudes, numVoices, numPartials) + otherVoicesDissonance;
                if (h_curr * h_prev < 0.0) {
                    intersectionPoint = p;
                    hit = true;
                    break;
                }
                h_prev = h_curr;
                if (t > 200.0f) break;
            }

            if (hit) {
                printf("Intersection Found!\n");
                printf("  Coefficients:  coeff_x=%.3f, coeff_z=%.3f\n", intersectionPoint.x, intersectionPoint.z);
                printf("  Frequencies:   f_x=%.2f Hz, f_z=%.2f Hz\n", voices[1].partials[0].freq * intersectionPoint.x, voices[2].partials[0].freq * intersectionPoint.z);
                printf("  Dissonance:    y=%.3f\n\n", intersectionPoint.y);
            }
        }

        /* --- Send data to shader and draw --- */
        Matrix view = GetCameraMatrix(camera);
        Matrix proj = MatrixPerspective(camera.fovy * DEG2RAD, (double)screenWidth / screenHeight, 0.1, 1000.0);
        Matrix invView = MatrixInvert(view);
        Matrix invProj = MatrixInvert(proj);


        SetShaderValueMatrix(shader, invViewLoc, invView);
        SetShaderValueMatrix(shader, invProjLoc, invProj);
        SetShaderValue(shader, cameraPosLoc, &camera.position, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, numVoicesLoc, &numVoices, SHADER_UNIFORM_INT);
        SetShaderValue(shader, numPartialsLoc, &numPartials, SHADER_UNIFORM_INT);
        SetShaderValueV(shader, voiceFreqsLoc, voiceFreqs, SHADER_UNIFORM_FLOAT, numVoices * numPartials);
        SetShaderValueV(shader, voiceAmplitudesLoc, voiceAmplitudes, SHADER_UNIFORM_FLOAT, numVoices * numPartials);
        SetShaderValue(shader, otherVoicesDissonanceLoc, &otherVoicesDissonance, SHADER_UNIFORM_FLOAT);
        float viewInts[] = {0.0, 0.0, (float)screenWidth, (float)screenHeight};
        SetShaderValue(shader, viewIntsLoc, &viewInts, SHADER_UNIFORM_VEC4);

        BeginTextureMode(target);
        ClearBackground(BLACK);
        BeginShaderMode(shader);
        DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
        EndShaderMode();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTextureRec(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height }, (Vector2){ 0, 0 }, WHITE);
        
        BeginMode2D(camera2d);
        // rlDisableDepthTest();
        DrawText("Dissonance Surface (Complex Tones)", 10, 10, 20, SKYBLUE);
        DrawText("RMB Click: Get Coords", 10, 40, 10, LIGHTGRAY);
        DrawFPS(screenWidth - 90, 10);
        EndMode2D();
        // rlEnableDepthTest();
        EndDrawing();
    }

    UnloadRenderTexture(target);
    UnloadShader(shader);
    CloseWindow();
    return 0;
}
