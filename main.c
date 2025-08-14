#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>
#include <stdio.h>
#define DISSONANCE_IMPLEMENTATION
#include "dissonance.h"
#include <OpenGL/gl.h>

/* ============================================================================ */
/*                            CORE DEFINITIONS                                  */
/* ============================================================================ */
#define MAX_PARTIALS 12
#define NUM_VOICES 3

RenderTexture2D LoadRenderTextureFloat(int width, int height) {
  RenderTexture2D target = {0};
  target.id = rlLoadFramebuffer();

  if (target.id > 0) {
    rlEnableFramebuffer(target.id);

    target.texture.id = rlLoadTexture(NULL, width, height, PIXELFORMAT_UNCOMPRESSED_R32, 1);
    target.texture.width = width;
    target.texture.height = height;
    target.texture.format = PIXELFORMAT_UNCOMPRESSED_R32;
    target.texture.mipmaps = 1;

    rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

    if (rlFramebufferComplete(target.id)) {
      TRACELOG(LOG_INFO, "FBO: [ID %i] Float framebuffer created successfully", target.id);
    }
    rlDisableFramebuffer();
  } else {
    TRACELOG(LOG_WARNING, "FBO: Framebuffer object could not be created");
  }
  return target;
}

int main(void) {
  /* --- Initialization --- */
  const int screenWidth = 1280;
  const int screenHeight = 720;
  InitWindow(screenWidth, screenHeight, "Dissonance Visualizer");
  SetTargetFPS(60);

  Camera cameraMesh = {0};
  cameraMesh.target = (Vector3){0.0f, 0.0f, 0.0f};
  cameraMesh.up = (Vector3){0.0f, 1.0f, 0.0f};
  cameraMesh.fovy = 45.0f;
  cameraMesh.projection = CAMERA_PERSPECTIVE;
  cameraMesh.position = (Vector3){5.0f, 5.0f, 5.0f};

  Camera2D camera2d = {0};
  camera2d.zoom = 1.0f;

  Shader bakingShader = LoadShader(0, "baking.fs");
  int baking_numVoicesLoc = GetShaderLocation(bakingShader, "numVoices");
  int baking_numPartialsLoc = GetShaderLocation(bakingShader, "numPartials");
  int baking_voiceFreqsLoc = GetShaderLocation(bakingShader, "voiceFreqs");
  int baking_voiceAmplitudesLoc = GetShaderLocation(bakingShader, "voiceAmplitudes");
  int baking_otherVoicesDissonanceLoc = GetShaderLocation(bakingShader, "otherVoicesDissonance");
  int baking_viewIntsLoc = GetShaderLocation(bakingShader, "viewInts");
  int baking_maxHeightLoc = GetShaderLocation(bakingShader, "maxHeight");

  Shader terrainShader = LoadShader("terrain.vs", "terrain.fs");
  int terrain_heightMultiplierLoc = GetShaderLocation(terrainShader, "heightMultiplier");
  int terrain_objectColorLoc = GetShaderLocation(terrainShader, "objectColor");
  int terrain_heightMapLoc = GetShaderLocation(terrainShader, "heightMap");

  RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
  RenderTexture2D heightmapTexture = LoadRenderTextureFloat(screenWidth, screenHeight);
  SetTextureFilter(heightmapTexture.texture, TEXTURE_FILTER_BILINEAR);
  SetTextureWrap(heightmapTexture.texture, TEXTURE_WRAP_CLAMP);

  Mesh terrainMesh = GenMeshPlane(4.0f, 4.0f, 256, 256);
  Material terrainMaterial = LoadMaterialDefault();
  terrainMaterial.shader = terrainShader;

  /* --- Voice Data Setup --- */
  int numVoices = NUM_VOICES;
  int numPartials = MAX_PARTIALS;
  Voice voices[NUM_VOICES] = {0};
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

  RenderMode currentMode = RENDER_MODE_RAYMARCH;
  float maxHeight = 1.0f;

  // --- Main Game Loop ---
  while (!WindowShouldClose()) {
    handle_input(&cameraMesh);
    // --- Update ---
    UpdateCameraPro(&cameraMesh,
                    (Vector3){IsKeyDown(KEY_W) * 0.1f - IsKeyDown(KEY_S) * 0.1f,
                              IsKeyDown(KEY_D) * 0.1f - IsKeyDown(KEY_A) * 0.1f,
                              IsKeyDown(KEY_R) * 0.1f - IsKeyDown(KEY_F) * 0.1f},
                    (Vector3){IsKeyDown(KEY_RIGHT) * 0.1f - IsKeyDown(KEY_LEFT) * 0.1f,
                              IsKeyDown(KEY_UP) * 0.1f - IsKeyDown(KEY_DOWN) * 0.1f, 0.0f},
                    GetMouseWheelMove() * 2.0f);

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
      Ray ray = GetMouseRay(GetMousePosition(), cameraMesh);
      bool hit = false;
      Vector3 intersectionPoint = {0};
      float t = 0.0f;
      Vector3 p = ray.position;
      float h_prev =
          p.y - getxzDissonance(p.x, p.z, voiceFreqs, voiceAmplitudes, numVoices, numPartials) + otherVoicesDissonance;

      for (int i = 0; i < 256; i++) {
        t += fmaxf(0.01f, h_prev * 0.5f);
        p = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
        float h_curr = p.y - getxzDissonance(p.x, p.z, voiceFreqs, voiceAmplitudes, numVoices, numPartials) +
                       otherVoicesDissonance;
        if (h_curr * h_prev < 0.0) {
          intersectionPoint = p;
          hit = true;
          break;
        }
        h_prev = h_curr;
        if (t > 200.0f)
          break;
      }

      if (hit) {
        printf("Intersection Found!\n");
        printf("  Coefficients:  coeff_x=%.3f, coeff_z=%.3f\n", intersectionPoint.x, intersectionPoint.z);
        printf("  Frequencies:   f_x=%.2f Hz, f_z=%.2f Hz\n", voices[1].partials[0].freq * intersectionPoint.x,
               voices[2].partials[0].freq * intersectionPoint.z);
        printf("  Dissonance:    y=%.3f\n\n", intersectionPoint.y);
      }
    }

    /* --- Pass 1: Bake heightmap (common for both modes) --- */
    BeginTextureMode(heightmapTexture);
    ClearBackground(BLANK);
    BeginShaderMode(bakingShader);
    SetShaderValue(bakingShader, baking_numVoicesLoc, &numVoices, SHADER_UNIFORM_INT);
    SetShaderValue(bakingShader, baking_numPartialsLoc, &numPartials, SHADER_UNIFORM_INT);
    SetShaderValueV(bakingShader, baking_voiceFreqsLoc, voiceFreqs, SHADER_UNIFORM_FLOAT, numVoices * numPartials);
    SetShaderValueV(bakingShader, baking_voiceAmplitudesLoc, voiceAmplitudes, SHADER_UNIFORM_FLOAT,
                    numVoices * numPartials);
    SetShaderValue(bakingShader, baking_otherVoicesDissonanceLoc, &otherVoicesDissonance, SHADER_UNIFORM_FLOAT);
    float bakingViewInts[] = {0.0, 0.0, (float)screenWidth, (float)screenHeight};
    SetShaderValue(bakingShader, baking_viewIntsLoc, &bakingViewInts, SHADER_UNIFORM_VEC4);
    SetShaderValue(bakingShader, baking_maxHeightLoc, &maxHeight, SHADER_UNIFORM_FLOAT);
    DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
    EndShaderMode();
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);

    BeginMode3D(cameraMesh);
    SetMaterialTexture(&terrainMaterial, MATERIAL_MAP_ALBEDO, heightmapTexture.texture);
    float heightMultiplier = 0.5f;
    SetShaderValueTexture(terrainShader, terrain_heightMapLoc, heightmapTexture.texture);
    SetShaderValue(terrainShader, terrain_heightMultiplierLoc, &heightMultiplier, SHADER_UNIFORM_FLOAT);
    Vector3 objectColor = {0.8f, 0.0f, 0.0f};
    SetShaderValue(terrainShader, terrain_objectColorLoc, &objectColor, SHADER_UNIFORM_VEC3);
    DrawMesh(terrainMesh, terrainMaterial, MatrixIdentity());
    DrawGrid(40, 0.1);
    EndMode3D();

    BeginMode2D(camera2d);
    DrawText("Press 'M' to switch render mode", 10, 10, 20, SKYBLUE);
    DrawText(TextFormat("Current Mode: %s", (currentMode == RENDER_MODE_RAYMARCH) ? "Raymarching" : "Mesh"), 10, 40, 10,
             LIGHTGRAY);
    DrawFPS(screenWidth - 90, 10);
    EndMode2D();
    EndDrawing();
  }

  UnloadRenderTexture(target);
  UnloadRenderTexture(heightmapTexture);
  UnloadShader(bakingShader);
  UnloadShader(terrainShader);
  UnloadMesh(terrainMesh);
  CloseWindow();
}
