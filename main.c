#include "dissonance.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/gl3.h>
#include <stdio.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

void handle_input(Camera3D *cameraMesh, Voices *voices, float otherVoicesDissonance, float worldPlaneSize) {
  UpdateCameraPro(cameraMesh,
                  (Vector3){IsKeyDown(KEY_W) * 0.1f - IsKeyDown(KEY_S) * 0.1f,
                            IsKeyDown(KEY_D) * 0.1f - IsKeyDown(KEY_A) * 0.1f,
                            IsKeyDown(KEY_R) * 0.1f - IsKeyDown(KEY_F) * 0.1f},
                  (Vector3){IsKeyDown(KEY_RIGHT) * 0.5f - IsKeyDown(KEY_LEFT) * 0.5f,
                            IsKeyDown(KEY_DOWN) * 0.5f - IsKeyDown(KEY_UP) * 0.5f, 0.0f},
                  GetMouseWheelMove() * 2.0f);

  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
    // Sample the terrain at mouse position - read-only operation
    Vector2 mousePos = GetMousePosition();
    Ray ray = GetMouseRay(mousePos, *cameraMesh);

    bool hit = false;
    Vector3 intersectionPoint = {0};
    float t = 0.0f;

    // Simple raymarching to find terrain intersection
    for (int i = 0; i < 1000; i++) {
      Vector3 p = Vector3Add(ray.position, Vector3Scale(ray.direction, t));

      // Get the terrain height at this XZ position
      float terrainHeight = get_xz_dissonance(voices, p.x, p.z, otherVoicesDissonance);

      // Check if ray height is below terrain height (considering height multiplier)
      if (p.y <= terrainHeight * 50.0f) { // 50.0f is the height multiplier used in rendering
        intersectionPoint = p;
        hit = true;
        break;
      }

      t += 0.1f; // Step size for raymarching
      if (t > 200.0f) {
        break; // Maximum ray distance
      }

      // Check if ray height is below terrain height (considering height multiplier)
      if (p.y <= terrainHeight * 50.0f) { // 50.0f is the height multiplier used in rendering
        intersectionPoint = p;
        hit = true;
        printf("Intersection found at step %d, t=%.2f\n", i, t);
        break;
      }

      t += 0.1f; // Step size for raymarching
      if (t > 200.0f) {
        printf("Raymarching reached maximum distance t=%.1f\n", t);
        break; // Maximum ray distance
      }
    }

    if (hit) {
      // Convert terrain coordinates (-2.0 to +2.0) to dissonance coordinates (0.0 to 4.0)
      float coeff_x = intersectionPoint.x + 0.5 * worldPlaneSize; // Convert from -2..+2 to 0..4
      float coeff_z = intersectionPoint.z + 0.5 * worldPlaneSize; // Convert from -2..+2 to 0..4
      float dissonance = get_xz_dissonance(voices, coeff_x, coeff_z, otherVoicesDissonance);

      printf("Terrain Sample (Read-Only):\n");
      printf("  Position:      x=%.3f, z=%.3f, y=%.3f\n", intersectionPoint.x, intersectionPoint.z,
             intersectionPoint.y);
      printf("  Coefficients:  coeff_x=%.3f, coeff_z=%.3f (converted to 0-4 range)\n", coeff_x, coeff_z);
      printf("  Frequencies:   f_x=%.2f Hz, f_z=%.2f Hz\n", voices->freqs[0] * coeff_x,
             voices->freqs[MAX_PARTIALS] * coeff_z);
      printf("  Dissonance:    %.6f\n", dissonance);
      printf("  Terrain Height: %.6f\n\n", get_xz_dissonance(voices, coeff_x, coeff_z, otherVoicesDissonance));
    } else {
      printf("No terrain intersection found.\n\n");
    }
  }
}

unsigned int set_up_grid(GLuint *terrainVAO, GLuint *terrainVBO, GLuint *terrainEBO, unsigned int meshResolution,
                         float worldPlaneSize) {
  glGenVertexArrays(1, terrainVAO);
  glGenBuffers(1, terrainVBO);
  glGenBuffers(1, terrainEBO);

  // Generate grid vertices
  int numVertices = (meshResolution + 1) * (meshResolution + 1);
  int numIndices = meshResolution * meshResolution * 6;

  float *vertices =
      (float *)malloc(numVertices * 8 * sizeof(float)); // 8 floats per vertex: pos(3) + normal(3) + texcoord(2)
  unsigned int *indices = (unsigned int *)malloc(numIndices * sizeof(unsigned int));

  // Generate vertices
  float step = worldPlaneSize / meshResolution;
  float halfSize = worldPlaneSize * 0.5f;
  int vertexIndex = 0;

  for (unsigned int z = 0; z <= meshResolution; z++) {
    for (unsigned int x = 0; x <= meshResolution; x++) {
      // Position (x, 0, z) - y will be sampled from texture in shader
      vertices[vertexIndex * 8 + 0] = x * step - halfSize; // x
      vertices[vertexIndex * 8 + 1] = 0.0f;                // y (will be displaced by shader)
      vertices[vertexIndex * 8 + 2] = z * step - halfSize; // z

      // Normal (will be calculated in shader)
      vertices[vertexIndex * 8 + 3] = 0.0f; // nx
      vertices[vertexIndex * 8 + 4] = 1.0f; // ny
      vertices[vertexIndex * 8 + 5] = 0.0f; // nz

      // Texture coordinates
      vertices[vertexIndex * 8 + 6] = (float)x / meshResolution; // u
      vertices[vertexIndex * 8 + 7] = (float)z / meshResolution; // v

      vertexIndex++;
    }
  }

  // Generate indices
  int indexIndex = 0;
  for (unsigned int z = 0; z < meshResolution; z++) {
    for (unsigned int x = 0; x < meshResolution; x++) {
      int topLeft = z * (meshResolution + 1) + x;
      int topRight = topLeft + 1;
      int bottomLeft = (z + 1) * (meshResolution + 1) + x;
      int bottomRight = bottomLeft + 1;

      // First triangle
      indices[indexIndex++] = topLeft;
      indices[indexIndex++] = bottomLeft;
      indices[indexIndex++] = topRight;

      // Second triangle
      indices[indexIndex++] = topRight;
      indices[indexIndex++] = bottomLeft;
      indices[indexIndex++] = bottomRight;
    }
  }

  // Upload to GPU
  glBindVertexArray(*terrainVAO);

  glBindBuffer(GL_ARRAY_BUFFER, *terrainVBO);
  glBufferData(GL_ARRAY_BUFFER, numVertices * 8 * sizeof(float), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *terrainEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), indices, GL_STATIC_DRAW);

  // Set up vertex attributes to match shader expectations
  // Position (location 0) - matches shader's aPos
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // Texture coordinates (location 1) - matches shader's aTexCoords
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Note: Normal (location 2) is not used by shader - it calculates normals from heightmap

  glBindVertexArray(0);

  // Clean up CPU memory
  free(vertices);
  free(indices);

  return numIndices;
}

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
  const int heightmapResolution = 1200;
  const float worldPlaneSize = 4.0f;
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
  int terrain_mvpLoc = GetShaderLocation(terrainShader, "mvp");
  int terrain_modelViewLoc = GetShaderLocation(terrainShader, "modelView");
  int terrain_normalMatrixLoc = GetShaderLocation(terrainShader, "normalMatrix");
  int terrain_lightPosLoc = GetShaderLocation(terrainShader, "lightPos");
  int terrain_textureSizeLoc = GetShaderLocation(terrainShader, "textureSize");
  int terrain_lightColorLoc = GetShaderLocation(terrainShader, "lightColor");
  int terrain_worldPlaneSizeLoc = GetShaderLocation(terrainShader, "worldPlaneSize");
  // int terrain_maxHeightLoc = GetShaderLocation(terrainShader, "maxHeight");

  RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
  RenderTexture2D heightmapTexture = LoadRenderTextureFloat(heightmapResolution, heightmapResolution);
  SetTextureFilter(heightmapTexture.texture, TEXTURE_FILTER_BILINEAR);
  SetTextureWrap(heightmapTexture.texture, TEXTURE_WRAP_CLAMP);

  const int meshResolution = 1200;

  // Create OpenGL buffers for terrain grid
  GLuint terrainVAO, terrainVBO, terrainEBO;
  unsigned int numIndices = set_up_grid(&terrainVAO, &terrainVBO, &terrainEBO, meshResolution, worldPlaneSize);

  /* --- Voice Data Setup --- */
  // voices really contain the spectra at base_freq
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
  Vector3 lightPos = {2.0f, 8.0f, 3.0f};

  float voice4 = 1.0;
  float voice5 = 1.0;

  while (!WindowShouldClose()) {
    voices.count = 3;
    generate_harmonic_series(&voices, base_freq * voice4, 1.0f, MAX_PARTIALS);
    generate_harmonic_series(&voices, base_freq * voice5, 1.0f, MAX_PARTIALS);
    float otherVoicesDissonance = calculate_dissonance(&voices, 2);
    handle_input(&cameraMesh, &voices, otherVoicesDissonance, worldPlaneSize);

    BeginTextureMode(heightmapTexture);
    ClearBackground(BLANK);
    BeginShaderMode(bakingShader);
    SetShaderValue(bakingShader, baking_numVoicesLoc, &voices.count, SHADER_UNIFORM_INT);
    SetShaderValue(bakingShader, baking_numPartialsLoc, &numPartials, SHADER_UNIFORM_INT);
    SetShaderValueV(bakingShader, baking_voiceFreqsLoc, voices.freqs, SHADER_UNIFORM_FLOAT, voices.count * numPartials);
    SetShaderValueV(bakingShader, baking_voiceAmplitudesLoc, voices.amps, SHADER_UNIFORM_FLOAT,
                    voices.count * numPartials);
    SetShaderValue(bakingShader, baking_otherVoicesDissonanceLoc, &otherVoicesDissonance, SHADER_UNIFORM_FLOAT);
    float bakingViewInts[] = {0.0, 0.0, (float)heightmapResolution, (float)heightmapResolution};
    SetShaderValue(bakingShader, baking_viewIntsLoc, &bakingViewInts, SHADER_UNIFORM_VEC4);
    SetShaderValue(bakingShader, baking_maxHeightLoc, &maxHeight, SHADER_UNIFORM_FLOAT);
    DrawRectangle(0, 0, heightmapResolution, heightmapResolution, WHITE);
    EndShaderMode();
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    // DrawTextureRec(heightmapTexture.texture, (Rectangle){ 0, 0, (float)heightmapTexture.texture.width,
    // (float)-heightmapTexture.texture.height }, (Vector2){ 0, 0 }, WHITE);

    BeginMode3D(cameraMesh);

    // Bind heightmap texture to texture unit 1
    rlActiveTextureSlot(1);
    glBindTexture(GL_TEXTURE_2D, heightmapTexture.texture.id);
    int heightMapLoc = GetShaderLocation(terrainShader, "heightMap");
    int textureUnit = 1;
    SetShaderValue(terrainShader, heightMapLoc, &textureUnit, SHADER_UNIFORM_INT);

    float heightMultiplier = 1.0f;
    SetShaderValue(terrainShader, terrain_heightMultiplierLoc, &heightMultiplier, SHADER_UNIFORM_FLOAT);

    Matrix modelView = GetCameraMatrix(cameraMesh);
    Matrix projection = rlGetMatrixProjection();
    Matrix mvp = MatrixMultiply(modelView, projection);
    Matrix normalMatrix = MatrixTranspose(MatrixInvert(modelView));

    Vector3 lightPosView = Vector3Transform(lightPos, modelView);

    SetShaderValueMatrix(terrainShader, terrain_mvpLoc, mvp);
    SetShaderValueMatrix(terrainShader, terrain_modelViewLoc, modelView);
    SetShaderValueMatrix(terrainShader, terrain_normalMatrixLoc, normalMatrix);
    SetShaderValue(terrainShader, terrain_lightPosLoc, &lightPosView, SHADER_UNIFORM_VEC3);
    float texSize = (float)heightmapResolution;
    SetShaderValue(terrainShader, terrain_textureSizeLoc, &texSize, SHADER_UNIFORM_FLOAT);
    Vector3 lightColor = {1.0f, 0.95f, 0.8f}; // Warm white light
    SetShaderValue(terrainShader, terrain_lightColorLoc, &lightColor, SHADER_UNIFORM_VEC3);
    float planeSize = worldPlaneSize;
    SetShaderValue(terrainShader, terrain_worldPlaneSizeLoc, &planeSize, SHADER_UNIFORM_FLOAT);

    // Render terrain using direct OpenGL
    glBindVertexArray(terrainVAO);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    DrawGrid(40, 0.1);
    EndMode3D();

    BeginMode2D(camera2d);
    GuiSlider((Rectangle){0.1f * screenWidth, 0.9f * screenHeight, 0.1f * screenWidth, 0.01 * screenHeight}, "voice 4",
              NULL, &voice4, 0.0f, 4.0f);
    GuiSlider((Rectangle){0.1f * screenWidth, 0.92f * screenHeight, 0.1f * screenWidth, 0.01 * screenHeight}, "voice 5",
              NULL, &voice5, 0.0f, 4.0f);
    DrawFPS(screenWidth - 90, 10);
    EndMode2D();
    EndDrawing();
  }

  UnloadRenderTexture(target);
  UnloadRenderTexture(heightmapTexture);
  UnloadShader(bakingShader);
  UnloadShader(terrainShader);

  // Clean up OpenGL resources
  glDeleteVertexArrays(1, &terrainVAO);
  glDeleteBuffers(1, &terrainVBO);
  glDeleteBuffers(1, &terrainEBO);

  CloseWindow();
}
