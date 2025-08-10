#ifndef VULKAN_BACKEND_H
#define VULKAN_BACKEND_H

#include <vulkan/vulkan.h>
#include "raylib.h"

// Match the layout of the shader's uniform block
typedef struct {
    Matrix invView;
    Matrix invProj;
    Vector3 cameraPos;
    int numVoices;
    int numPartials;
    float otherVoicesDissonance;
    float voiceFreqs[32];
    float voiceAmplitudes[32];
    Vector4 viewInts;
} Uniforms;

void vulkan_init(void* windowHandle);
void vulkan_draw_frame(const Uniforms* uniforms);
void vulkan_cleanup();

#endif // VULKAN_BACKEND_H
