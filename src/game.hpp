#pragma once

#include "engine.hpp"

#include <memory>

class GameRuntime {
public:
	GameRuntime();
	~GameRuntime();

	GameRuntime(const GameRuntime&) = delete;
	GameRuntime& operator=(const GameRuntime&) = delete;

	int ScreenWidth() const;
	int ScreenHeight() const;

	bool ShouldExitRequested() const;

	void LoadSettingsFromFile();
	void ApplyLoadedSettings();
	void LoadAssets();
	void UnloadAssets();
	void Update(const engine::FrameContext& frame);
	void Draw() const;

private:
	class Impl;
	std::unique_ptr<Impl> impl;
};
