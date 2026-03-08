#include "raylib.h"

int main() {
    InitWindow(800, 450, "raylib smoke test");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Raylib works", 300, 210, 30, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}