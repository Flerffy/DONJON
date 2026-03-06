#include "raylib.h"

int main() {
    constexpr int kScreenWidth = 1280;
    constexpr int kScreenHeight = 720;

    InitWindow(kScreenWidth, kScreenHeight, "DONJON");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("DONJON + raylib", 40, 40, 32, RAYWHITE);
        DrawText("Press ESC to quit", 40, 90, 22, GRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
