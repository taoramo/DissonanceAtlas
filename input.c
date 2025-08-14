#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include "dissonance.h"
#include <stdio.h>

void handle_input(Camera3D *cameraMesh, Voices *voices, float otherVoicesDissonance) {
  UpdateCameraPro(
      cameraMesh,
      (Vector3){
          IsKeyDown(KEY_W) * 0.1f - IsKeyDown(KEY_S) * 0.1f,
          IsKeyDown(KEY_D) * 0.1f - IsKeyDown(KEY_A) * 0.1f,
          IsKeyDown(KEY_R) * 0.1f - IsKeyDown(KEY_F) * 0.1f
      },
      (Vector3){
          IsKeyDown(KEY_RIGHT) * 0.1f - IsKeyDown(KEY_LEFT) * 0.1f,
          IsKeyDown(KEY_UP) * 0.1f - IsKeyDown(KEY_DOWN) * 0.1f,
          0.0f
      },
      GetMouseWheelMove() * 2.0f);

  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
      Ray ray = GetMouseRay(GetMousePosition(), *cameraMesh);
      bool hit = false;
      Vector3 intersectionPoint = { 0 };
      float t = 0.0f;
      Vector3 p = ray.position;
      float h_prev = p.y - get_xz_dissonance(voices,p.x, p.y, otherVoicesDissonance);

      for (int i = 0; i < 256; i++) {
          t += fmaxf(0.01f, h_prev * 0.5f);
          p = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
          float h_curr = p.y - get_xz_dissonance(voices,p.x, p.z, otherVoicesDissonance) + otherVoicesDissonance;
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
          printf("  Frequencies:   f_x=%.2f Hz, f_z=%.2f Hz\n", voices->freqs[0] * intersectionPoint.x, voices->freqs[MAX_PARTIALS] * intersectionPoint.z);
          printf("  Dissonance:    y=%.3f\n\n", intersectionPoint.y);
      }
  }
}
