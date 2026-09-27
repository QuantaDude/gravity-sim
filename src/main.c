#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include <raylib-nuklear.h>
#include <raylib.h>

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(1920, 1080, "raylib-nuklear example");
  // Create the Nuklear Context
  int fontSize = 13;
  struct nk_context *ctx = InitNuklear(fontSize);

  while (!WindowShouldClose()) {
    // Update the Nuklear context, along with input
    UpdateNuklear(ctx);

    // Nuklear GUI Code
    // https://github.com/Immediate-Mode-UI/Nuklear/wiki/Window
    if (nk_begin(ctx, "Nuklear", nk_rect(0, 0, 220, 1080),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE)) {
      nk_layout_row_static(ctx, 50, 150, 1);
      if (nk_button_label(ctx, "Button")) {
        // Button was clicked!
      }
    }
    nk_end(ctx);

    // Render
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawCircle(640 / 2, 480 / 2, 50.0f, GREEN);
    // Render the Nuklear GUI
    DrawNuklear(ctx);

    EndDrawing();
  }

  // De-initialize the Nuklear GUI
  UnloadNuklear(ctx);

  CloseWindow();
  return 0;
}
