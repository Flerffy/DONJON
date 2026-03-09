#pragma once

#include "raylib.h"

#include <cstdint>
#include <array>
#include <functional>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace engine {

struct EngineConfig {
    int virtualWidth = 1280;
    int virtualHeight = 720;
    int targetFps = 60;
    bool resizableWindow = true;
    std::string windowTitle;
};

struct FrameContext {
    float deltaTime = 0.0f;
    int windowWidth = 0;
    int windowHeight = 0;
    int renderWidth = 0;
    int renderHeight = 0;
    int offsetX = 0;
    int offsetY = 0;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
};

struct EngineCallbacks {
    std::function<void()> onStartup;
    std::function<void(const FrameContext&)> onUpdate;
    std::function<void(const FrameContext&)> onDraw;
    std::function<bool()> shouldExit;
    std::function<void()> onShutdown;
};

struct InputSnapshot {
    bool escapePressed = false;
    bool tabPressed = false;
    bool interactPressed = false;
    bool consoleTogglePressed = false;
    bool restartPressed = false;
    bool mouseLeftPressed = false;
    bool mouseRightPressed = false;
    bool moveUpPressed = false;
    bool moveDownPressed = false;
    bool moveLeftPressed = false;
    bool moveRightPressed = false;
    std::array<bool, 10> skillSlotPressed{};
};

enum class MoveIntent : uint8_t {
    None,
    Up,
    Down,
    Left,
    Right
};

using GridOffset = std::pair<int, int>;
const std::array<GridOffset, 4>& CardinalOffsets();
const std::array<GridOffset, 8>& SurroundingOffsets();
std::vector<GridOffset> SquareAreaOffsets(int radius, bool includeCenter = true);
std::vector<GridOffset> DiamondAreaOffsets(int radius, bool includeCenter = true);
std::vector<GridOffset> PlusAreaOffsets(int radius, bool includeCenter = true);
std::vector<GridOffset> ForwardArc3Offsets(const GridOffset& direction);
bool HasNeighborMatching(
    int x,
    int y,
    int width,
    int height,
    const std::function<bool(int, int)>& predicate,
    bool includeDiagonals = true);

bool InBounds(int x, int y, int width, int height);
int GridIndex(int x, int y, int width);
int ManhattanDistance(int x1, int y1, int x2, int y2);
GridOffset UnitStepToward(int fromX, int fromY, int toX, int toY);
GridOffset DominantAxisStepToward(int fromX, int fromY, int toX, int toY);
float SmoothStep01(float t);
float ExponentialSmoothingAlpha(float smoothing, float deltaTime);
Vector2 LerpVector2(Vector2 from, Vector2 to, float alpha);
std::vector<std::pair<int, int>> TraceRayTiles(
    int startX,
    int startY,
    int stepX,
    int stepY,
    int width,
    int height,
    const std::function<bool(int, int)>& isOpaque,
    int maxSteps = -1);
std::vector<std::pair<int, int>> TraceRayToward(
    int fromX,
    int fromY,
    int toX,
    int toY,
    int width,
    int height,
    const std::function<bool(int, int)>& isOpaque,
    int maxSteps = -1);
bool HasPathBfs(
    int width,
    int height,
    int startX,
    int startY,
    int goalX,
    int goalY,
    const std::function<bool(int, int)>& isWalkable);

class TurnClock {
public:
    void TickFrame(float deltaTime);
    void AdvanceTurn();

    uint64_t FrameCount() const;
    uint64_t TurnCount() const;
    float ElapsedSeconds() const;

private:
    uint64_t frameCount = 0;
    uint64_t turnCount = 0;
    float elapsedSeconds = 0.0f;
};

struct Rect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

class Rng {
public:
    using result_type = std::mt19937::result_type;

    explicit Rng(uint32_t seed = std::random_device{}());

    int NextInt(int minValue, int maxValue);
    bool RollChance(float chance);

    result_type operator()();
    static constexpr result_type min() { return std::mt19937::min(); }
    static constexpr result_type max() { return std::mt19937::max(); }

private:
    std::mt19937 generator;
};

std::vector<std::pair<int, int>> FindPathAStar(
    int width,
    int height,
    int startX,
    int startY,
    int goalX,
    int goalY,
    const std::function<bool(int, int)>& isWalkable);

bool HasLineOfSight(
    int startX,
    int startY,
    int endX,
    int endY,
    int width,
    int height,
    const std::function<bool(int, int)>& isOpaque);

std::vector<uint8_t> ComputeFovMask(
    int width,
    int height,
    int originX,
    int originY,
    int radius,
    const std::function<bool(int, int)>& isOpaque);

bool RoomsOverlap(const Rect& a, const Rect& b);
std::pair<int, int> RoomCenter(const Rect& room);

void CarveRoom(const Rect& room, const std::function<void(int, int)>& setFloor);
void CarveCorridor(int x1, int y1, int x2, int y2, const std::function<void(int, int)>& setFloor);

void BuildWallsFromOutside(
    int width,
    int height,
    const std::function<bool(int, int)>& isOutside,
    const std::function<bool(int, int)>& isInterior,
    const std::function<void(int, int, bool)>& setWall);

void Run(const EngineConfig& config, const EngineCallbacks& callbacks);
InputSnapshot PollInput();
MoveIntent ReadCardinalMove(const InputSnapshot& input);

bool TryLoadTextureFromFile(const char* path, Texture2D& texture, bool& loaded);
void SafeUnloadTexture(Texture2D& texture, bool& loaded);

bool TryLoadSoundFromFile(const char* path, Sound& sound, bool& loaded, float baseVolume = 1.0f);
void SafeUnloadSound(Sound& sound, bool& loaded);

bool TryLoadMusicFromFile(const char* path, Music& music, bool& loaded);
void SafeUnloadMusic(Music& music, bool& loaded);

} // namespace engine
