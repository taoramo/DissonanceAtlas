#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "dissonance.h"
#include <OpenGL/gl.h>
#include <stdio.h>

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
  if (!IsShaderValid(bakingShader)) {
    TraceLog(LOG_ERROR, "Failed to load baking shader");
    return 1;
  }
  int baking_numVoicesLoc = GetShaderLocation(bakingShader, "numVoices");
  int baking_numPartialsLoc = GetShaderLocation(bakingShader, "numPartials");
  int baking_voiceFreqsLoc = GetShaderLocation(bakingShader, "voiceFreqs");
  int baking_voiceAmplitudesLoc = GetShaderLocation(bakingShader, "voiceAmplitudes");
  int baking_otherVoicesDissonanceLoc = GetShaderLocation(bakingShader, "otherVoicesDissonance");
  int baking_viewIntsLoc = GetShaderLocation(bakingShader, "viewInts");
  int baking_maxHeightLoc = GetShaderLocation(bakingShader, "maxHeight");

  Shader terrainShader = LoadShader("terrain.vs", "terrain.fs");
  if (!IsShaderValid(terrainShader)) {
    TraceLog(LOG_ERROR, "Failed to load terrain shader");
    return 1;
  }
  int terrain_heightMultiplierLoc = GetShaderLocation(terrainShader, "heightMultiplier");
  int terrain_objectColorLoc = GetShaderLocation(terrainShader, "objectColor");
  int terrain_heightMapLoc = GetShaderLocation(terrainShader, "heightMap");
  // int terrain_maxHeightLoc = GetShaderLocation(terrainShader, "maxHeight");

  RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
  RenderTexture2D heightmapTexture = LoadRenderTextureFloat(screenWidth, screenHeight);
  SetTextureFilter(heightmapTexture.texture, TEXTURE_FILTER_BILINEAR);
  SetTextureWrap(heightmapTexture.texture, TEXTURE_WRAP_CLAMP);

  Mesh terrainMesh = GenMeshPlane(4.0f, 4.0f, 256, 256);
  Material terrainMaterial = LoadMaterialDefault();
  terrainMaterial.shader = terrainShader;

  /* --- Voice Data Setup --- */
  Voices voices = {0};
  int numPartials = MAX_PARTIALS;
  float base_freq = 220.0f;
  generate_harmonic_series(&voices, base_freq, 1.0f, MAX_PARTIALS);
  generate_harmonic_series(&voices, base_freq, 1.0f, MAX_PARTIALS);
  generate_harmonic_series(&voices, base_freq, 1.0f, MAX_PARTIALS);
  // for (int i = 0; i < voices.count * numPartials; i++) {
  //   printf("freq: %f, amp: %f, i: %d\n", voices.freqs[i], voices.amps[i], i);
  // }

  float maxHeight = 1.0f;

  while (!WindowShouldClose()) {
    float otherVoicesDissonance = calculate_dissonance(&voices);
    // printf("otherVoicesDissonance: %f", otherVoicesDissonance);
    handle_input(&cameraMesh, &voices, otherVoicesDissonance);

        BeginTextureMode(heightmapTexture);
        ClearBackground(BLANK);
        BeginShaderMode(bakingShader);
        SetShaderValue(bakingShader, baking_numVoicesLoc, &voices.count, SHADER_UNIFORM_INT);
        SetShaderValue(bakingShader, baking_numPartialsLoc, &numPartials, SHADER_UNIFORM_INT);
        SetShaderValueV(bakingShader, baking_voiceFreqsLoc, voices.freqs, SHADER_UNIFORM_FLOAT, voices.count * numPartials);
        SetShaderValueV(bakingShader, baking_voiceAmplitudesLoc, voices.amps, SHADER_UNIFORM_FLOAT, voices.count * numPartials);
        SetShaderValue(bakingShader, baking_otherVoicesDissonanceLoc, &otherVoicesDissonance, SHADER_UNIFORM_FLOAT);
        float bakingViewInts[] = {0.0, 0.0, (float)screenWidth, (float)screenHeight};
        SetShaderValue(bakingShader, baking_viewIntsLoc, &bakingViewInts, SHADER_UNIFORM_VEC4);
        SetShaderValue(bakingShader, baking_maxHeightLoc, &maxHeight, SHADER_UNIFORM_FLOAT);
        DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
        EndShaderMode();
        EndTextureMode();

    BeginDrawing(); 
    ClearBackground(BLACK);
    DrawTextureRec(heightmapTexture.texture, (Rectangle){ 0, 0, (float)heightmapTexture.texture.width, (float)-heightmapTexture.texture.height }, (Vector2){ 0, 0 }, WHITE);

            // BeginMode3D(cameraMesh);
            // SetMaterialTexture(&terrainMaterial, MATERIAL_MAP_HEIGHT, heightmapTexture.texture);
            // float heightMultiplier = 0.5f;
            // //SetShaderValueTexture(terrainShader, terrain_heightMapLoc, heightmapTexture.texture);
            // SetShaderValue(terrainShader, terrain_heightMultiplierLoc, &heightMultiplier, SHADER_UNIFORM_FLOAT);
            // Vector3 objectColor = { 0.8f, 0.8f, 0.8f };
            // SetShaderValue(terrainShader, terrain_objectColorLoc, &objectColor, SHADER_UNIFORM_VEC3);
            // // DrawMesh(terrainMesh, terrainMaterial, MatrixIdentity());
            // DrawGrid(40, 0.1);
            // EndMode3D();

      BeginMode2D(camera2d);
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
