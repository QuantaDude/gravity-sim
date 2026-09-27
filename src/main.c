#include <math.h>
#include <stdio.h>
#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include <raylib-nuklear.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

struct Objects {
  Vector2 pos;
};

int main() {

  long double worldSpace_x = 0.0L;
  long double worldSpace_y = 0.0L;

  double G = 6.674f;

  printf("%lf", G);
  Vector2 pos1 = {0.0f, 0.0f};
  Vector2 pos2 = {500.0f, 500.0f};
  float mass1 = 1000.0f;
  float mass2 = 100.0f;

  Vector2 vel1 = {1.5f, 0.0f};
  Vector2 vel2 = {0.0f, 1.5f};

  Vector2 F21 = {0, 0};

  // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(1920, 1080, "raylib-nuklear example");

  Camera2D camera = {.zoom = 1.0f,
                     .offset = {.x = 0.0f, .y = 0.0f},
                     .rotation = 0.0f,
                     .target = {.x = 0.0f, .y = 0.0f}};
  // Create the Nuklear Context
  int fontSize = 13;
  struct nk_context *ctx = InitNuklear(fontSize);

  SetTargetFPS(60);
  // char *FPSstring = NULL;
  float timeScale = 1.0f;

  while (!WindowShouldClose()) {
    // delta
    Vector2 displacementVect = Vector2Subtract(pos1, pos2);
    float displaceMagnitudeSq = Vector2LengthSqr(displacementVect);

    float forceMagnitude = (G * mass1 * mass2) / displaceMagnitudeSq;

    Vector2 direction =
        Vector2Scale(displacementVect, 1.0f / Vector2Length(displacementVect));

    Vector2 F21 = Vector2Scale(direction, forceMagnitude);

    Vector2 a1 = Vector2Scale(F21, 1.0f / mass2);
    Vector2 a2 = Vector2Scale(F21, -1.0f / mass1);

    float dt = GetFrameTime();

    vel1 = Vector2Add(vel1, Vector2Scale(a1, dt * timeScale));
    vel2 = Vector2Add(vel2, Vector2Scale(a2, dt * timeScale));

    pos2 = Vector2Add(pos2, Vector2Scale(vel1, dt * timeScale));
    pos1 = Vector2Add(pos1, Vector2Scale(vel2, dt * timeScale));
    // sprintf(FPSstring, "%d\0", GetFPS());
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

      camera.offset = GetMousePosition();

      camera.target = mouseWorldPos;

      float scale = 0.2f * wheel;
      camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.000125f, 64.0f);
    }
    UpdateNuklear(ctx);

    // Nuklear GUI Code
    // https://github.com/Immediate-Mode-UI/Nuklear/wiki/Window
    if (nk_begin(ctx, "Nuklear", nk_rect(0, 0, 220, 1080),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE)) {
      nk_layout_row_static(ctx, 50, 220, 1);
      nk_label(ctx, "Time Scale", NK_TEXT_CENTERED);
      timeScale = nk_slide_float(ctx, 0.01f, timeScale, 200.0f, 5.0f);
      nk_property_float(ctx, "planet 1 mass", 0.00001f, &mass1, 100000000000.0f,
                        0.5f, 1.0f);
      nk_property_float(ctx, "planet 2 mass", 0.00001f, &mass2, 100000000000.0f,
                        0.5f, 1.0f);
      if (nk_button_label(ctx, "Button")) {
        // Button was clicked!
      }
    }
    nk_end(ctx);

    // Render
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode2D(camera);
    rlPushMatrix();
    rlTranslatef(0, 20 * 20, 0);
    rlRotatef(90, 1, 0, 0);
    DrawGrid(10000 * camera.zoom, 50 * (1.0f / camera.zoom));

    rlPopMatrix();
    DrawCircleV(pos1, 50.0f, RED);
    DrawCircleV(pos2, 50.0f, GREEN);
    DrawText("planet 1", pos1.x, pos1.y - 70, 12.0f, RED);
    DrawText("planet 2", pos2.x, pos2.y - 70, 12.0f, GREEN);
    EndMode2D();
    // Render the Nuklear GUI
    DrawNuklear(ctx);

    EndDrawing();

    // printf("%f, %f", pos1.x, pos1.y);
  }

  // De-initialize the Nuklear GUI
  UnloadNuklear(ctx);

  CloseWindow();
  return 0;
}

// F21 = Vector2Scale(
//     Vector2Normalize(
//         Vector2Scale(Vector2Subtract(pos2, pos1),
//                      Vector2Length(Vector2Subtract(pos2, pos1)))),
//     -G * ((mass1 * mass2) / Vector2LengthSqr(Vector2Subtract(pos2,
//     pos1))));
