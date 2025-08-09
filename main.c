#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>
#define DISSONANCE_IMPLEMENTATION
#include "dissonance.h"
#include <OpenGL/gl.h>


/* ============================================================================ */
/*                            CORE DEFINITIONS                                  */
/* ============================================================================ */
#define MAX_PARTIALS 6
#define NUM_VOICES 3

RenderTexture2D LoadRenderTextureFloat(int width, int height) {
    RenderTexture2D target = {0};
    target.id = rlLoadFramebuffer();

    if (target.id > 0) {
        rlEnableFramebuffer(target.id);

        target.texture.id = rlLoadTexture(
            NULL, width, height, PIXELFORMAT_UNCOMPRESSED_R32, 1
        );
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R32;
        target.texture.mipmaps = 1;

        rlFramebufferAttach(
            target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0,
            RL_ATTACHMENT_TEXTURE2D, 0
        );

        if (rlFramebufferComplete(target.id)) {
            TRACELOG(
                LOG_INFO,
                "FBO: [ID %i] Float framebuffer created successfully", target.id
            );
        }
        rlDisableFramebuffer();
    } else {
        TRACELOG(LOG_WARNING, "FBO: Framebuffer object could not be created");
    }
    return target;
}

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
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position = (Vector3){ 4.0f, 3.0f, 4.0f };

    Camera2D camera2d = { 0 };
    camera2d.zoom = 1.0f;

    /* --- Shader and Uniforms --- */
    Shader shader = LoadShader(0, "dissonance.fs");
    int invViewLoc = GetShaderLocation(shader, "invView");
    int invProjLoc = GetShaderLocation(shader, "invProj");
    int cameraPosLoc = GetShaderLocation(shader, "cameraPos");
    int viewIntsLoc = GetShaderLocation(shader, "viewInts");
    int heightmapLoc = GetShaderLocation(shader, "heightmap");
    int surfaceDimensionsLoc = GetShaderLocation(shader, "surfaceDimensions");

    Shader bakingShader = LoadShader(0, "baking.fs");
    int baking_numVoicesLoc = GetShaderLocation(bakingShader, "numVoices");
    int baking_numPartialsLoc = GetShaderLocation(bakingShader, "numPartials");
    int baking_voiceFreqsLoc = GetShaderLocation(bakingShader, "voiceFreqs");
    int baking_voiceAmplitudesLoc = GetShaderLocation(bakingShader, "voiceAmplitudes");
    int baking_otherVoicesDissonanceLoc = GetShaderLocation(bakingShader, "otherVoicesDissonance");
    int baking_viewIntsLoc = GetShaderLocation(bakingShader, "viewInts");

    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    RenderTexture2D heightmapTexture = LoadRenderTexture(screenWidth, screenHeight);

    // Use the high-level raylib functions to set the texture parameters.
    // We must use a filter that does not require mipmaps.
    // BILINEAR is equivalent to GL_LINEAR.
    SetTextureFilter(heightmapTexture.texture, RL_TEXTURE_FILTER_BILINEAR);
    // Set wrapping to clamp, which prevents weird artifacts at the edges.
    SetTextureWrap(heightmapTexture.texture, RL_TEXTURE_WRAP_CLAMP);

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
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            // Get the vector from the target to the camera
            Vector3 toCamera = Vector3Subtract(camera.position, camera.target);
            float distance = Vector3Length(toCamera);
            // Scale distance by zoom factor, but prevent going inside the target
            distance = fmaxf(distance - wheel * 0.8f, 0.1f);
            // Recalculate position
            camera.position = Vector3Add(camera.target, Vector3Scale(Vector3Normalize(toCamera), distance));
        }

        // Orbit with left mouse button
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mouseDelta = GetMouseDelta();
            float rotateSpeed = 0.005f; // Adjust sensitivity

            // Get the vector from the target to the camera
            Vector3 toCamera = Vector3Subtract(camera.position, camera.target);

            // Horizontal rotation (around the world's Y axis)
            toCamera = Vector3RotateByAxisAngle(toCamera, camera.up, -mouseDelta.x * rotateSpeed);

            // Vertical rotation (around the camera's "right" axis)
            Vector3 right = Vector3CrossProduct(Vector3Normalize(toCamera), camera.up);
            toCamera = Vector3RotateByAxisAngle(toCamera, right, -mouseDelta.y * rotateSpeed);

            // Update camera position
            camera.position = Vector3Add(camera.target, toCamera);
        }      

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

        // --- Pass 1: Bake heightmap ---
        BeginTextureMode(heightmapTexture);
        ClearBackground(BLANK);
        BeginShaderMode(bakingShader);
        // Set baking shader uniforms
        SetShaderValue(bakingShader, baking_numVoicesLoc, &numVoices, SHADER_UNIFORM_INT);
        SetShaderValue(bakingShader, baking_numPartialsLoc, &numPartials, SHADER_UNIFORM_INT);
        SetShaderValueV(bakingShader, baking_voiceFreqsLoc, voiceFreqs, SHADER_UNIFORM_FLOAT, numVoices * numPartials);
        SetShaderValueV(bakingShader, baking_voiceAmplitudesLoc, voiceAmplitudes, SHADER_UNIFORM_FLOAT, numVoices * numPartials);
        SetShaderValue(bakingShader, baking_otherVoicesDissonanceLoc, &otherVoicesDissonance, SHADER_UNIFORM_FLOAT);
        float bakingViewInts[] = {0.0, 0.0, (float)screenWidth, (float)screenHeight};
        SetShaderValue(bakingShader, baking_viewIntsLoc, &bakingViewInts, SHADER_UNIFORM_VEC4);

        DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
        EndShaderMode();
        EndTextureMode();
        Image tmp = LoadImageFromTexture(heightmapTexture.texture);
        UpdateTexture(heightmapTexture.texture, tmp.data);

        // --- Pass 2: Render scene ---
        SetShaderValueMatrix(shader, invViewLoc, invView);
        SetShaderValueMatrix(shader, invProjLoc, invProj);
        SetShaderValue(shader, cameraPosLoc, &camera.position, SHADER_UNIFORM_VEC3);
        SetShaderValueTexture(shader, heightmapLoc, heightmapTexture.texture);
        float surfaceDimensions[] = {4.0, 4.0};
        SetShaderValue(shader, surfaceDimensionsLoc, &surfaceDimensions, SHADER_UNIFORM_VEC2);
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
        DrawText("Dissonance Surface (Complex Tones)", 10, 10, 20, SKYBLUE);
        DrawText("RMB Click: Get Coords", 10, 40, 10, LIGHTGRAY);
        DrawFPS(screenWidth - 90, 10);
        EndMode2D();
        EndDrawing();
    }

    UnloadRenderTexture(target);
    UnloadRenderTexture(heightmapTexture);
    UnloadShader(shader);
    UnloadShader(bakingShader);
    CloseWindow();
    return 0;
}
