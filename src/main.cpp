#include "engine.hpp"
#include "game.hpp"

int main() {
    GameRuntime game;

    engine::EngineConfig engineConfig;
    engineConfig.virtualWidth = game.ScreenWidth();
    engineConfig.virtualHeight = game.ScreenHeight();
    engineConfig.targetFps = 60;
    engineConfig.resizableWindow = true;
    engineConfig.windowTitle = "DONJON - Roguelite Prototype";

    engine::EngineCallbacks callbacks;
    callbacks.onStartup = [&game]() {
        game.LoadSettingsFromFile();
        game.ApplyLoadedSettings();
        game.LoadAssets();
    };
    callbacks.onUpdate = [&game](const engine::FrameContext& frame) {
        game.Update(frame);
    };
    callbacks.onDraw = [&game](const engine::FrameContext&) {
        game.Draw();
    };
    callbacks.shouldExit = [&game]() {
        return game.ShouldExitRequested();
    };
    callbacks.onShutdown = [&game]() {
        game.UnloadAssets();
    };

    engine::Run(engineConfig, callbacks);
    return 0;
}
