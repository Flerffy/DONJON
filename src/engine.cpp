#include "engine.hpp"

#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace engine {

const std::array<GridOffset, 4>& CardinalOffsets() {
    static const std::array<GridOffset, 4> offsets{{
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    }};
    return offsets;
}

const std::array<GridOffset, 8>& SurroundingOffsets() {
    static const std::array<GridOffset, 8> offsets{{
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    }};
    return offsets;
}

std::vector<GridOffset> SquareAreaOffsets(int radius, bool includeCenter) {
    std::vector<GridOffset> offsets;
    int clampedRadius = std::max(0, radius);
    int side = clampedRadius * 2 + 1;
    offsets.reserve(static_cast<size_t>(side * side));

    for (int dy = -clampedRadius; dy <= clampedRadius; ++dy) {
        for (int dx = -clampedRadius; dx <= clampedRadius; ++dx) {
            if (!includeCenter && dx == 0 && dy == 0) continue;
            offsets.push_back({dx, dy});
        }
    }
    return offsets;
}

std::vector<GridOffset> DiamondAreaOffsets(int radius, bool includeCenter) {
    std::vector<GridOffset> offsets;
    int clampedRadius = std::max(0, radius);
    int side = clampedRadius * 2 + 1;
    offsets.reserve(static_cast<size_t>(side * side));

    for (int dy = -clampedRadius; dy <= clampedRadius; ++dy) {
        for (int dx = -clampedRadius; dx <= clampedRadius; ++dx) {
            if (!includeCenter && dx == 0 && dy == 0) continue;
            if (std::abs(dx) + std::abs(dy) > clampedRadius) continue;
            offsets.push_back({dx, dy});
        }
    }
    return offsets;
}

std::vector<GridOffset> PlusAreaOffsets(int radius, bool includeCenter) {
    std::vector<GridOffset> offsets;
    int clampedRadius = std::max(0, radius);
    offsets.reserve(static_cast<size_t>(clampedRadius * 4 + (includeCenter ? 1 : 0)));

    if (includeCenter) {
        offsets.push_back({0, 0});
    }

    for (int d = 1; d <= clampedRadius; ++d) {
        offsets.push_back({d, 0});
        offsets.push_back({-d, 0});
        offsets.push_back({0, d});
        offsets.push_back({0, -d});
    }
    return offsets;
}

std::vector<GridOffset> ForwardArc3Offsets(const GridOffset& direction) {
    if (direction.first == 1 && direction.second == 0) {
        return {{1, -1}, {1, 0}, {1, 1}};
    }
    if (direction.first == -1 && direction.second == 0) {
        return {{-1, -1}, {-1, 0}, {-1, 1}};
    }
    if (direction.first == 0 && direction.second == 1) {
        return {{-1, 1}, {0, 1}, {1, 1}};
    }
    if (direction.first == 0 && direction.second == -1) {
        return {{-1, -1}, {0, -1}, {1, -1}};
    }
    return {};
}

bool HasNeighborMatching(
    int x,
    int y,
    int width,
    int height,
    const std::function<bool(int, int)>& predicate,
    bool includeDiagonals) {
    if (!InBounds(x, y, width, height)) return false;

    if (includeDiagonals) {
        for (const auto& offset : SurroundingOffsets()) {
            int nx = x + offset.first;
            int ny = y + offset.second;
            if (!InBounds(nx, ny, width, height)) continue;
            if (predicate(nx, ny)) return true;
        }
        return false;
    }

    for (const auto& offset : CardinalOffsets()) {
        int nx = x + offset.first;
        int ny = y + offset.second;
        if (!InBounds(nx, ny, width, height)) continue;
        if (predicate(nx, ny)) return true;
    }
    return false;
}

bool InBounds(int x, int y, int width, int height) {
    return x >= 0 && x < width && y >= 0 && y < height;
}

int GridIndex(int x, int y, int width) {
    return y * width + x;
}

int ManhattanDistance(int x1, int y1, int x2, int y2) {
    return std::abs(x2 - x1) + std::abs(y2 - y1);
}

GridOffset UnitStepToward(int fromX, int fromY, int toX, int toY) {
    int dx = toX - fromX;
    int dy = toY - fromY;
    int stepX = (dx > 0) ? 1 : (dx < 0 ? -1 : 0);
    int stepY = (dy > 0) ? 1 : (dy < 0 ? -1 : 0);
    return {stepX, stepY};
}

GridOffset DominantAxisStepToward(int fromX, int fromY, int toX, int toY) {
    int relX = toX - fromX;
    int relY = toY - fromY;
    if (relX == 0 && relY == 0) return {0, 0};

    if (std::abs(relX) >= std::abs(relY)) {
        return {(relX > 0) ? 1 : -1, 0};
    }
    return {0, (relY > 0) ? 1 : -1};
}

float SmoothStep01(float t) {
    float clamped = std::clamp(t, 0.0f, 1.0f);
    return clamped * clamped * (3.0f - 2.0f * clamped);
}

float ExponentialSmoothingAlpha(float smoothing, float deltaTime) {
    float safeSmoothing = std::max(0.0f, smoothing);
    float safeDelta = std::max(0.0f, deltaTime);
    return 1.0f - std::exp(-safeSmoothing * safeDelta);
}

Vector2 LerpVector2(Vector2 from, Vector2 to, float alpha) {
    float t = std::clamp(alpha, 0.0f, 1.0f);
    return Vector2{
        from.x + (to.x - from.x) * t,
        from.y + (to.y - from.y) * t
    };
}

std::vector<std::pair<int, int>> TraceRayTiles(
    int startX,
    int startY,
    int stepX,
    int stepY,
    int width,
    int height,
    const std::function<bool(int, int)>& isOpaque,
    int maxSteps) {
    std::vector<std::pair<int, int>> tiles;
    if (stepX == 0 && stepY == 0) return tiles;

    int x = startX + stepX;
    int y = startY + stepY;
    int steps = 0;
    while (InBounds(x, y, width, height) && !isOpaque(x, y) && (maxSteps < 0 || steps < maxSteps)) {
        tiles.push_back({x, y});
        x += stepX;
        y += stepY;
        steps += 1;
    }
    return tiles;
}

std::vector<std::pair<int, int>> TraceRayToward(
    int fromX,
    int fromY,
    int toX,
    int toY,
    int width,
    int height,
    const std::function<bool(int, int)>& isOpaque,
    int maxSteps) {
    GridOffset step = UnitStepToward(fromX, fromY, toX, toY);
    return TraceRayTiles(fromX, fromY, step.first, step.second, width, height, isOpaque, maxSteps);
}

bool HasPathBfs(
    int width,
    int height,
    int startX,
    int startY,
    int goalX,
    int goalY,
    const std::function<bool(int, int)>& isWalkable) {
    if (!InBounds(startX, startY, width, height) || !InBounds(goalX, goalY, width, height)) return false;
    if (startX == goalX && startY == goalY) return true;

    std::vector<uint8_t> visited(static_cast<size_t>(width) * static_cast<size_t>(height), 0);
    std::vector<std::pair<int, int>> queue;
    queue.reserve(static_cast<size_t>(width) * static_cast<size_t>(height));

    visited[static_cast<size_t>(GridIndex(startX, startY, width))] = 1;
    queue.push_back({startX, startY});

    size_t head = 0;
    while (head < queue.size()) {
        int x = queue[head].first;
        int y = queue[head].second;
        ++head;

        for (const auto& offset : CardinalOffsets()) {
            int nx = x + offset.first;
            int ny = y + offset.second;
            if (!InBounds(nx, ny, width, height)) continue;
            if (!isWalkable(nx, ny)) continue;

            int idx = GridIndex(nx, ny, width);
            if (visited[static_cast<size_t>(idx)] != 0) continue;
            if (nx == goalX && ny == goalY) return true;

            visited[static_cast<size_t>(idx)] = 1;
            queue.push_back({nx, ny});
        }
    }

    return false;
}

void TurnClock::TickFrame(float deltaTime) {
    frameCount += 1;
    elapsedSeconds += std::max(0.0f, deltaTime);
}

void TurnClock::AdvanceTurn() {
    turnCount += 1;
}

uint64_t TurnClock::FrameCount() const {
    return frameCount;
}

uint64_t TurnClock::TurnCount() const {
    return turnCount;
}

float TurnClock::ElapsedSeconds() const {
    return elapsedSeconds;
}

Rng::Rng(uint32_t seed)
    : generator(seed) {
}

int Rng::NextInt(int minValue, int maxValue) {
    std::uniform_int_distribution<int> dist(minValue, maxValue);
    return dist(generator);
}

bool Rng::RollChance(float chance) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(generator) < chance;
}

Rng::result_type Rng::operator()() {
    return generator();
}

std::vector<std::pair<int, int>> FindPathAStar(
    int width,
    int height,
    int startX,
    int startY,
    int goalX,
    int goalY,
    const std::function<bool(int, int)>& isWalkable) {
    std::vector<std::pair<int, int>> empty;
    if (width <= 0 || height <= 0) return empty;

    auto inBounds = [width, height](int x, int y) {
        return x >= 0 && x < width && y >= 0 && y < height;
    };
    auto indexAt = [width](int x, int y) {
        return y * width + x;
    };
    auto heuristic = [](int x1, int y1, int x2, int y2) {
        return std::abs(x1 - x2) + std::abs(y1 - y2);
    };

    if (!inBounds(startX, startY) || !inBounds(goalX, goalY)) return empty;
    if (!isWalkable(startX, startY) || !isWalkable(goalX, goalY)) return empty;

    const int nodeCount = width * height;
    const int inf = std::numeric_limits<int>::max();

    std::vector<int> gScore(nodeCount, inf);
    std::vector<int> fScore(nodeCount, inf);
    std::vector<int> cameFrom(nodeCount, -1);
    std::vector<bool> open(nodeCount, false);
    std::vector<bool> closed(nodeCount, false);

    int start = indexAt(startX, startY);
    int goal = indexAt(goalX, goalY);

    gScore[start] = 0;
    fScore[start] = heuristic(startX, startY, goalX, goalY);
    open[start] = true;

    while (true) {
        int current = -1;
        int bestF = inf;

        for (int i = 0; i < nodeCount; ++i) {
            if (open[i] && !closed[i] && fScore[i] < bestF) {
                bestF = fScore[i];
                current = i;
            }
        }

        if (current == -1) return empty;
        if (current == goal) break;

        open[current] = false;
        closed[current] = true;

        int currentX = current % width;
        int currentY = current / width;

        for (const auto& offset : CardinalOffsets()) {
            int nextX = currentX + offset.first;
            int nextY = currentY + offset.second;
            if (!inBounds(nextX, nextY) || !isWalkable(nextX, nextY)) continue;

            int nextIndex = indexAt(nextX, nextY);
            if (closed[nextIndex]) continue;

            int tentativeG = gScore[current] + 1;
            if (tentativeG < gScore[nextIndex]) {
                cameFrom[nextIndex] = current;
                gScore[nextIndex] = tentativeG;
                fScore[nextIndex] = tentativeG + heuristic(nextX, nextY, goalX, goalY);
                open[nextIndex] = true;
            }
        }
    }

    std::vector<std::pair<int, int>> path;
    for (int current = goal; current != -1; current = cameFrom[current]) {
        int x = current % width;
        int y = current / width;
        path.push_back({x, y});
        if (current == start) break;
    }

    std::reverse(path.begin(), path.end());
    return path;
}

bool HasLineOfSight(
    int startX,
    int startY,
    int endX,
    int endY,
    int width,
    int height,
    const std::function<bool(int, int)>& isOpaque) {
    auto inBounds = [width, height](int x, int y) {
        return x >= 0 && x < width && y >= 0 && y < height;
    };

    int dx = std::abs(endX - startX);
    int sx = (startX < endX) ? 1 : -1;
    int dy = -std::abs(endY - startY);
    int sy = (startY < endY) ? 1 : -1;
    int err = dx + dy;

    int x = startX;
    int y = startY;
    while (true) {
        if (!(x == endX && y == endY)) {
            if (!inBounds(x, y)) return false;
            if (isOpaque(x, y)) return false;
        }

        if (x == endX && y == endY) break;

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }

    return true;
}

std::vector<uint8_t> ComputeFovMask(
    int width,
    int height,
    int originX,
    int originY,
    int radius,
    const std::function<bool(int, int)>& isOpaque) {
    std::vector<uint8_t> visible;
    if (width <= 0 || height <= 0) return visible;

    visible.assign(static_cast<size_t>(width) * static_cast<size_t>(height), 0);
    auto inBounds = [width, height](int x, int y) {
        return x >= 0 && x < width && y >= 0 && y < height;
    };
    auto idx = [width](int x, int y) {
        return y * width + x;
    };

    if (!inBounds(originX, originY)) return visible;

    visible[static_cast<size_t>(idx(originX, originY))] = 1;

    const int clampedRadius = std::max(0, radius);
    const int minX = std::max(0, originX - clampedRadius);
    const int maxX = std::min(width - 1, originX + clampedRadius);
    const int minY = std::max(0, originY - clampedRadius);
    const int maxY = std::min(height - 1, originY + clampedRadius);
    const int radiusSq = clampedRadius * clampedRadius;

    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            const int dx = x - originX;
            const int dy = y - originY;
            if (dx * dx + dy * dy > radiusSq) continue;

            if (HasLineOfSight(originX, originY, x, y, width, height, isOpaque)) {
                visible[static_cast<size_t>(idx(x, y))] = 1;
            }
        }
    }

    return visible;
}

bool RoomsOverlap(const Rect& a, const Rect& b) {
    return a.x <= (b.x + b.w) && (a.x + a.w) >= b.x && a.y <= (b.y + b.h) && (a.y + a.h) >= b.y;
}

std::pair<int, int> RoomCenter(const Rect& room) {
    return {room.x + room.w / 2, room.y + room.h / 2};
}

void CarveRoom(const Rect& room, const std::function<void(int, int)>& setFloor) {
    for (int y = room.y; y < room.y + room.h; ++y) {
        for (int x = room.x; x < room.x + room.w; ++x) {
            setFloor(x, y);
        }
    }
}

void CarveCorridor(int x1, int y1, int x2, int y2, const std::function<void(int, int)>& setFloor) {
    int x = x1;
    int y = y1;

    while (x != x2) {
        setFloor(x, y);
        x += (x2 > x) ? 1 : -1;
    }

    while (y != y2) {
        setFloor(x, y);
        y += (y2 > y) ? 1 : -1;
    }

    setFloor(x2, y2);
}

void BuildWallsFromOutside(
    int width,
    int height,
    const std::function<bool(int, int)>& isOutside,
    const std::function<bool(int, int)>& isInterior,
    const std::function<void(int, int, bool)>& setWall) {
    auto inBounds = [width, height](int x, int y) {
        return x >= 0 && x < width && y >= 0 && y < height;
    };

    std::vector<std::pair<std::pair<int, int>, bool>> changes;
    changes.reserve(static_cast<size_t>(width) * static_cast<size_t>(height));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (!isOutside(x, y)) continue;

            bool adjacentToInterior = false;
            for (const auto& offset : SurroundingOffsets()) {
                int nx = x + offset.first;
                int ny = y + offset.second;
                if (!inBounds(nx, ny)) continue;
                if (isInterior(nx, ny)) {
                    adjacentToInterior = true;
                    break;
                }
            }

            if (!adjacentToInterior) continue;

            bool floorBelow = false;
            if (y + 1 < height) {
                floorBelow = isInterior(x, y + 1);
            }
            changes.push_back({{x, y}, floorBelow});
        }
    }

    for (const auto& change : changes) {
        setWall(change.first.first, change.first.second, change.second);
    }
}

void Run(const EngineConfig& config, const EngineCallbacks& callbacks) {
    if (config.resizableWindow) {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    }

    InitWindow(config.virtualWidth, config.virtualHeight, config.windowTitle.c_str());
    SetWindowMinSize(config.virtualWidth, config.virtualHeight);
    InitAudioDevice();
    SetExitKey(KEY_NULL);
    SetTargetFPS(config.targetFps);

    RenderTexture2D gameTarget = LoadRenderTexture(config.virtualWidth, config.virtualHeight);
    SetTextureFilter(gameTarget.texture, TEXTURE_FILTER_POINT);

    if (callbacks.onStartup) {
        callbacks.onStartup();
    }

    while (!WindowShouldClose() && !(callbacks.shouldExit && callbacks.shouldExit())) {
        FrameContext frame{};
        frame.deltaTime = GetFrameTime();
        frame.windowWidth = GetScreenWidth();
        frame.windowHeight = GetScreenHeight();
        frame.renderWidth = frame.windowWidth;
        frame.renderHeight = frame.windowHeight;
        frame.offsetX = 0;
        frame.offsetY = 0;
        frame.scaleX = static_cast<float>(frame.renderWidth) / static_cast<float>(config.virtualWidth);
        frame.scaleY = static_cast<float>(frame.renderHeight) / static_cast<float>(config.virtualHeight);
        frame.scaleX = std::max(0.01f, frame.scaleX);
        frame.scaleY = std::max(0.01f, frame.scaleY);

        SetMouseOffset(-frame.offsetX, -frame.offsetY);
        SetMouseScale(1.0f / frame.scaleX, 1.0f / frame.scaleY);

        if (callbacks.onUpdate) {
            callbacks.onUpdate(frame);
        }

        BeginTextureMode(gameTarget);
        ClearBackground(BLACK);
        if (callbacks.onDraw) {
            callbacks.onDraw(frame);
        }
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        Rectangle src{0.0f, 0.0f, static_cast<float>(gameTarget.texture.width), -static_cast<float>(gameTarget.texture.height)};
        Rectangle dst{static_cast<float>(frame.offsetX), static_cast<float>(frame.offsetY), static_cast<float>(frame.renderWidth), static_cast<float>(frame.renderHeight)};
        DrawTexturePro(gameTarget.texture, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        EndDrawing();
    }

    if (callbacks.onShutdown) {
        callbacks.onShutdown();
    }

    UnloadRenderTexture(gameTarget);
    CloseAudioDevice();
    CloseWindow();
}

InputSnapshot PollInput() {
    InputSnapshot input;
    input.escapePressed = IsKeyPressed(KEY_ESCAPE);
    input.tabPressed = IsKeyPressed(KEY_TAB);
    input.interactPressed = IsKeyPressed(KEY_E);
    input.consoleTogglePressed = IsKeyPressed(KEY_GRAVE);
    input.restartPressed = IsKeyPressed(KEY_R);
    input.mouseLeftPressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    input.mouseRightPressed = IsMouseButtonPressed(MOUSE_RIGHT_BUTTON);
    input.moveUpPressed = IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
    input.moveDownPressed = IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN);
    input.moveLeftPressed = IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT);
    input.moveRightPressed = IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT);

    input.skillSlotPressed[0] = IsKeyPressed(KEY_ONE);
    input.skillSlotPressed[1] = IsKeyPressed(KEY_TWO);
    input.skillSlotPressed[2] = IsKeyPressed(KEY_THREE);
    input.skillSlotPressed[3] = IsKeyPressed(KEY_FOUR);
    input.skillSlotPressed[4] = IsKeyPressed(KEY_FIVE);
    input.skillSlotPressed[5] = IsKeyPressed(KEY_SIX);
    input.skillSlotPressed[6] = IsKeyPressed(KEY_SEVEN);
    input.skillSlotPressed[7] = IsKeyPressed(KEY_EIGHT);
    input.skillSlotPressed[8] = IsKeyPressed(KEY_NINE);
    input.skillSlotPressed[9] = IsKeyPressed(KEY_ZERO);
    return input;
}

MoveIntent ReadCardinalMove(const InputSnapshot& input) {
    if (input.moveUpPressed) return MoveIntent::Up;
    if (input.moveDownPressed) return MoveIntent::Down;
    if (input.moveLeftPressed) return MoveIntent::Left;
    if (input.moveRightPressed) return MoveIntent::Right;
    return MoveIntent::None;
}

bool TryLoadTextureFromFile(const char* path, Texture2D& texture, bool& loaded) {
    if (loaded || path == nullptr) return loaded;
    if (!FileExists(path)) return false;

    texture = LoadTexture(path);
    loaded = texture.id > 0;
    if (!loaded) {
        texture = Texture2D{};
    }
    return loaded;
}

void SafeUnloadTexture(Texture2D& texture, bool& loaded) {
    if (!loaded) return;
    UnloadTexture(texture);
    texture = Texture2D{};
    loaded = false;
}

bool TryLoadSoundFromFile(const char* path, Sound& sound, bool& loaded, float baseVolume) {
    if (loaded || path == nullptr) return loaded;
    if (!FileExists(path)) return false;

    sound = LoadSound(path);
    loaded = sound.frameCount > 0;
    if (!loaded) {
        sound = Sound{};
        return false;
    }

    SetSoundVolume(sound, baseVolume);
    return true;
}

void SafeUnloadSound(Sound& sound, bool& loaded) {
    if (!loaded) return;
    UnloadSound(sound);
    sound = Sound{};
    loaded = false;
}

bool TryLoadMusicFromFile(const char* path, Music& music, bool& loaded) {
    if (loaded || path == nullptr) return loaded;
    if (!FileExists(path)) return false;

    music = LoadMusicStream(path);
    loaded = music.ctxData != nullptr;
    if (!loaded) {
        music = Music{};
    }
    return loaded;
}

void SafeUnloadMusic(Music& music, bool& loaded) {
    if (!loaded) return;
    StopMusicStream(music);
    UnloadMusicStream(music);
    music = Music{};
    loaded = false;
}

} // namespace engine
