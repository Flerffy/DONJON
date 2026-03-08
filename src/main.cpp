#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include <limits>
#include <array>
#include <optional>
#include <cstddef>
#include <cctype>

enum class TileType : uint8_t {
    Outside,
    Wall,
    Floor,
    Stairs,
};

enum class PlayerClass : uint8_t {
    Warrior,
    Ranger,
    Wizard
};

enum class ItemCategory : uint8_t {
    Equipment,
    Consumable
};

enum class EquipSlot : uint8_t {
    Head,
    Top,
    Feet,
    Hands,
    Ring1,
    Ring2,
    Necklace,
    Weapon,
    Shield,
    Count
};

enum class ConsumableType : uint8_t {
    HealthPotion,
    ArmorPotion,
    Antidote,
    Bandage
};

enum class ItemTier : uint8_t {
    Common,
    Uncommon,
    Rare,
    Mythic,
    Legendary
};

enum class WeaponType : uint8_t {
    Sword,
    Bow,
    Staff
};

enum class EnemyArchetype : uint8_t {
    GoblinWarrior,
    GoblinRanger,
    GoblinWizard,
    UndeadWarrior,
    UndeadRanger,
    UndeadWizard,
    Rat,
    Wolf,
    Bat
};

enum class EnemyTelegraph : uint8_t {
    None,
    RangerLine,
    WizardPlus
};

enum class GoldStackType : uint8_t {
    Small,
    Medium,
    Large
};

enum class MerchantTab : uint8_t {
    Buy,
    Reroll,
    Sell
};

enum class InteractableType : uint8_t {
    Chest,
    Pot,
    Button,
    Spike
};

enum class ActiveSkillId : uint8_t {
    None,
    WarriorCleave,
    WarriorSecondWind,
    WarriorShieldWall,
    RangerPiercingShot,
    RangerVolley,
    RangerSurvivalInstinct,
    WizardArcaneNova,
    WizardBlink,
    WizardBarrier
};

enum class PassiveStatType : uint8_t {
    MaxHp,
    Strength,
    Dexterity,
    Intelligence,
    Armor,
    Damage,
    GoldDropped,
    ExpEarned,
    EnemySpawned
};

enum class PassiveSuffixId : uint8_t {
    Vampirism,
    Larceny,
    Punishment,
    Fortress,
    Echo,
    Discovery,
    Enlightenment,
    Trickery,
    Revival,
    Contagion,
    Conduit,
    Charity,
    Miasma,
    Radiance
};

struct GeneratedPassive {
    PassiveStatType statType = PassiveStatType::MaxHp;
    bool increasesStat = true;
    ItemTier grade = ItemTier::Common;
    PassiveSuffixId suffix = PassiveSuffixId::Vampirism;
    float statPercent = 0.0f;
    float passivePercent = 0.0f;
};

struct Item {
    std::string name;
    ItemCategory category = ItemCategory::Equipment;
    ItemTier tier = ItemTier::Common;
    int itemLevel = 1;
    int basePower = 0;
    float rarityMultiplier = 1.0f;
    float itemPower = 0.0f;

    EquipSlot slot = EquipSlot::Weapon;
    WeaponType weaponType = WeaponType::Sword;
    int hpBonus = 0;
    int armorBonus = 0;
    int strBonus = 0;
    int dexBonus = 0;
    int intBonus = 0;
    int primaryStatType = -1; // 0=STR, 1=DEX, 2=INT

    ConsumableType consumableType = ConsumableType::HealthPotion;
    bool stackable = false;
    int quantity = 1;
};

struct Enemy {
    int x = 0;
    int y = 0;
    int hp = 0;
    int maxHp = 0;
    int atk = 0;
    bool alive = true;
    bool panicFlee = false;

    bool seesPlayer = false;   // NEW: red "!" state
    bool isFleeingNow = false; // NEW: yellow "!!!" flashing state
    int iconVariant = 0;
    EnemyArchetype archetype = EnemyArchetype::GoblinWarrior;
    EnemyTelegraph telegraph = EnemyTelegraph::None;
    int telegraphDx = 0;
    int telegraphDy = 0;
    int telegraphTargetX = 0;
    int telegraphTargetY = 0;
    int staffCharge = 5;
    bool isFlying = false;
};

struct Player {
    int x = 0;
    int y = 0;
    int hp = 20;
    int maxHp = 20;
    int atk = 5;

    int level = 1;
    int xp = 0;
    int xpToNext = 10;

    PlayerClass playerClass = PlayerClass::Warrior;

    int baseStrength = 5;
    int baseDexterity = 3;
    int baseIntelligence = 2;
    int baseMaxHp = 20;

    int strength = 5;
    int dexterity = 3;
    int intelligence = 2;
    int armor = 0;
    int staffCharge = 5;
    int gold = 0;
    bool poisoned = false;
    bool bleeding = false;
    int skillPoints = 0;
    std::array<int, 3> classSkillRanks{0, 0, 0};
    std::array<ActiveSkillId, 10> activeSkillLoadout{
        ActiveSkillId::None, ActiveSkillId::None, ActiveSkillId::None, ActiveSkillId::None, ActiveSkillId::None,
        ActiveSkillId::None, ActiveSkillId::None, ActiveSkillId::None, ActiveSkillId::None, ActiveSkillId::None
    };
    std::array<int, 10> activeSkillCooldowns{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int temporaryArmorBonus = 0;
    int temporaryArmorTurns = 0;
    int temporaryAttackBonus = 0;
    int temporaryAttackTurns = 0;
    std::vector<GeneratedPassive> passiveSkills;

    std::vector<Item> inventory;
    std::array<std::optional<Item>, static_cast<size_t>(EquipSlot::Count)> equipped{};
};

struct Room {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

struct GoldStack {
    int x = 0;
    int y = 0;
    int amount = 0;
    GoldStackType type = GoldStackType::Small;
};

struct Decoration {
    int x = 0;
    int y = 0;
    int variant = 0;
};

struct Interactable {
    int x = 0;
    int y = 0;
    InteractableType type = InteractableType::Chest;
    bool opened = false;
};

std::vector<std::string> WrapTextStrict(const std::string& text, int maxWidth, int fontSize) {
    std::vector<std::string> lines;
    std::string currentLine;

    for (char c : text) {
        if (c == '\n') {
            lines.push_back(currentLine);
            currentLine.clear();
            continue;
        }

        std::string test = currentLine + c;
        if (!currentLine.empty() && MeasureText(test.c_str(), fontSize) > maxWidth) {
            lines.push_back(currentLine);
            currentLine.clear();
        }
        currentLine += c;
    }

    if (!currentLine.empty()) lines.push_back(currentLine);
    return lines;
}

class Game {
public:
    static constexpr int kTileSize = 28;
    static constexpr int kMapWidth = 36;
    static constexpr int kMapHeight = 24;
    static constexpr int kHudWidth = 272;
    static constexpr int kScreenWidth = (kMapWidth * kTileSize) + kHudWidth;
    static constexpr int kScreenHeight = kMapHeight * kTileSize;

    void LoadAssets() {
        if (!roomAtlasLoaded) {
            roomAtlas = LoadTexture("assets/icons/room_atlas.png");
            roomAtlasLoaded = roomAtlas.id > 0;
        }
        if (!actionAtlasLoaded) {
            actionAtlas = LoadTexture("assets/icons/action_atlas.png");
            actionAtlasLoaded = actionAtlas.id > 0;
        }
        if (!walkablesAtlasLoaded) {
            walkablesAtlas = LoadTexture("assets/icons/walkables_atlas.png");
            walkablesAtlasLoaded = walkablesAtlas.id > 0;
        }
        if (!solidsAtlasLoaded) {
            solidsAtlas = LoadTexture("assets/icons/solids_atlas.png");
            solidsAtlasLoaded = solidsAtlas.id > 0;
        }
        if (!stairsIconLoaded) {
            stairsIcon = LoadTexture("assets/icons/stairs.png");
            stairsIconLoaded = stairsIcon.id > 0;
        }
        if (!iconAtlasLoaded) {
            iconAtlas = LoadTexture("assets/icons/item_atlas.png");
            iconAtlasLoaded = iconAtlas.id > 0;
        }
        if (!playerAtlasLoaded) {
            playerAtlas = LoadTexture("assets/icons/player_atlas.png");
            playerAtlasLoaded = playerAtlas.id > 0;
        }
        if (!enemyAtlasLoaded) {
            enemyAtlas = LoadTexture("assets/icons/enemy_atlas.png");
            enemyAtlasLoaded = enemyAtlas.id > 0;
        }
        if (!npcAtlasLoaded) {
            npcAtlas = LoadTexture("assets/icons/npc_atlas.png");
            npcAtlasLoaded = npcAtlas.id > 0;
        }

        if (!musicLoaded) {
            const char* bgmPath = FileExists("assets/audio/Goblins_Den_(Regular).wav")
                ? "assets/audio/Goblins_Den_(Regular).wav"
                : (FileExists("assets/audio/bgm.ogg") ? "assets/audio/bgm.ogg" : nullptr);
            if (bgmPath != nullptr) {
                musicBgm = LoadMusicStream(bgmPath);
                musicLoaded = musicBgm.ctxData != nullptr;
                if (musicLoaded) {
                    SetMusicVolume(musicBgm, 0.45f);
                    PlayMusicStream(musicBgm);
                }
            }
        }

        LoadSfxIfExists("assets/audio/26_sword_hit_1.wav", sfxAttack, sfxAttackLoaded, 0.55f);
        LoadSfxIfExists("assets/audio/26_sword_hit_2.wav", sfxAttack2, sfxAttack2Loaded, 0.55f);
        LoadSfxIfExists("assets/audio/26_sword_hit_3.wav", sfxAttack3, sfxAttack3Loaded, 0.55f);
        LoadSfxIfExists("assets/audio/35_Miss_Evade_02.wav", sfxAttackRanged, sfxAttackRangedLoaded, 0.50f);
        LoadSfxIfExists("assets/audio/04_Fire_explosion_04_medium.wav", sfxAttackMagic, sfxAttackMagicLoaded, 0.50f);
        LoadSfxIfExists("assets/audio/21_orc_damage_1.wav", sfxEnemyDamaged, sfxEnemyDamagedLoaded, 0.55f);
        LoadSfxIfExists("assets/audio/21_orc_damage_2.wav", sfxEnemyDamaged2, sfxEnemyDamaged2Loaded, 0.55f);
        LoadSfxIfExists("assets/audio/21_orc_damage_3.wav", sfxEnemyDamaged3, sfxEnemyDamaged3Loaded, 0.55f);
        LoadSfxIfExists("assets/audio/11_human_damage_1.wav", sfxHurt, sfxHurtLoaded, 0.60f);
        LoadSfxIfExists("assets/audio/11_human_damage_2.wav", sfxHurt2, sfxHurt2Loaded, 0.60f);
        LoadSfxIfExists("assets/audio/11_human_damage_3.wav", sfxHurt3, sfxHurt3Loaded, 0.60f);
        LoadSfxIfExists("assets/audio/08_Bite_04.wav", sfxBeastAttack, sfxBeastAttackLoaded, 0.60f);
        LoadSfxIfExists("assets/audio/03_Claw_03.wav", sfxBeastAttack2, sfxBeastAttack2Loaded, 0.60f);
        LoadSfxIfExists("assets/audio/079_Buy_sell_01.wav", sfxGold, sfxGoldLoaded, 0.55f);
        LoadSfxIfExists("assets/audio/013_Confirm_03.wav", sfxInteract, sfxInteractLoaded, 0.55f);
        LoadSfxIfExists("assets/audio/051_use_item_01.wav", sfxUseItem, sfxUseItemLoaded, 0.55f);
        LoadSfxIfExists("assets/audio/029_Decline_09.wav", sfxDecline, sfxDeclineLoaded, 0.55f);
        LoadSfxIfExists("assets/audio/092_Pause_04.wav", sfxMenuOpen, sfxMenuOpenLoaded, 0.55f);
        LoadSfxIfExists("assets/audio/098_Unpause_04.wav", sfxMenuClose, sfxMenuCloseLoaded, 0.55f);
        LoadSfxIfExists("assets/audio/05_door_open_1.mp3", sfxStairs, sfxStairsLoaded, 0.65f);
        LoadSfxIfExists("assets/audio/01_chest_open_1.wav", sfxChestOpen, sfxChestOpenLoaded, 0.60f);
        LoadSfxIfExists("assets/audio/01_chest_open_2.wav", sfxChestOpen2, sfxChestOpen2Loaded, 0.60f);
        LoadSfxIfExists("assets/audio/01_chest_open_3.wav", sfxChestOpen3, sfxChestOpen3Loaded, 0.60f);
        LoadSfxIfExists("assets/audio/01_chest_open_4.wav", sfxChestOpen4, sfxChestOpen4Loaded, 0.60f);
        LoadSfxIfExists("assets/audio/69_Enemy_death_01.wav", sfxDeath, sfxDeathLoaded, 0.70f);
    }

    void UnloadAssets() {
        if (roomAtlasLoaded) {
            UnloadTexture(roomAtlas);
            roomAtlasLoaded = false;
            roomAtlas = Texture2D{};
        }
        if (actionAtlasLoaded) {
            UnloadTexture(actionAtlas);
            actionAtlasLoaded = false;
            actionAtlas = Texture2D{};
        }
        if (walkablesAtlasLoaded) {
            UnloadTexture(walkablesAtlas);
            walkablesAtlasLoaded = false;
            walkablesAtlas = Texture2D{};
        }
        if (solidsAtlasLoaded) {
            UnloadTexture(solidsAtlas);
            solidsAtlasLoaded = false;
            solidsAtlas = Texture2D{};
        }
        if (stairsIconLoaded) {
            UnloadTexture(stairsIcon);
            stairsIconLoaded = false;
            stairsIcon = Texture2D{};
        }
        if (iconAtlasLoaded) {
            UnloadTexture(iconAtlas);
            iconAtlasLoaded = false;
            iconAtlas = Texture2D{};
        }
        if (playerAtlasLoaded) {
            UnloadTexture(playerAtlas);
            playerAtlasLoaded = false;
            playerAtlas = Texture2D{};
        }
        if (enemyAtlasLoaded) {
            UnloadTexture(enemyAtlas);
            enemyAtlasLoaded = false;
            enemyAtlas = Texture2D{};
        }
        if (npcAtlasLoaded) {
            UnloadTexture(npcAtlas);
            npcAtlasLoaded = false;
            npcAtlas = Texture2D{};
        }

        if (musicLoaded) {
            StopMusicStream(musicBgm);
            UnloadMusicStream(musicBgm);
            musicLoaded = false;
            musicBgm = Music{};
        }

        UnloadSfx(sfxAttack, sfxAttackLoaded);
        UnloadSfx(sfxAttack2, sfxAttack2Loaded);
        UnloadSfx(sfxAttack3, sfxAttack3Loaded);
        UnloadSfx(sfxAttackRanged, sfxAttackRangedLoaded);
        UnloadSfx(sfxAttackMagic, sfxAttackMagicLoaded);
        UnloadSfx(sfxEnemyDamaged, sfxEnemyDamagedLoaded);
        UnloadSfx(sfxEnemyDamaged2, sfxEnemyDamaged2Loaded);
        UnloadSfx(sfxEnemyDamaged3, sfxEnemyDamaged3Loaded);
        UnloadSfx(sfxHurt, sfxHurtLoaded);
        UnloadSfx(sfxHurt2, sfxHurt2Loaded);
        UnloadSfx(sfxHurt3, sfxHurt3Loaded);
        UnloadSfx(sfxBeastAttack, sfxBeastAttackLoaded);
        UnloadSfx(sfxBeastAttack2, sfxBeastAttack2Loaded);
        UnloadSfx(sfxGold, sfxGoldLoaded);
        UnloadSfx(sfxInteract, sfxInteractLoaded);
        UnloadSfx(sfxUseItem, sfxUseItemLoaded);
        UnloadSfx(sfxDecline, sfxDeclineLoaded);
        UnloadSfx(sfxMenuOpen, sfxMenuOpenLoaded);
        UnloadSfx(sfxMenuClose, sfxMenuCloseLoaded);
        UnloadSfx(sfxStairs, sfxStairsLoaded);
        UnloadSfx(sfxChestOpen, sfxChestOpenLoaded);
        UnloadSfx(sfxChestOpen2, sfxChestOpen2Loaded);
        UnloadSfx(sfxChestOpen3, sfxChestOpen3Loaded);
        UnloadSfx(sfxChestOpen4, sfxChestOpen4Loaded);
        UnloadSfx(sfxDeath, sfxDeathLoaded);
    }

    void StartNewRun() {
        dungeonLevel = 1;
        nextMerchantFloor = RandomInt(3, 5);
        score = 0;
        gameOver = false;
        player = Player{};
        combatLog.clear();
        merchantStock.clear();
        merchantPresent = false;
        merchantOpen = false;
        merchantX = -1;
        merchantY = -1;
        merchantDoorX = -1;
        merchantDoorY = -1;
        merchantRoomOpened = false;
        merchantIconVariant = 0;
        merchantTab = MerchantTab::Buy;
        merchantSelection = 0;

        classSelectionPending = true;
        inventoryOpen = false;
        inventorySelection = 0;
        inventoryScrollOffset = 0;
        pauseMenuOpen = false;
        devConsoleOpen = false;
        devConsoleInput.clear();

        // Keep render-safe data before class is chosen
        tiles.assign(kMapWidth * kMapHeight, TileType::Outside);
        enemies.clear();
        goldStacks.clear();
        walkableDecorations.clear();
        solidDecorations.clear();
        interactables.clear();
        rooms.clear();
        passiveChoiceOpen = false;
        passiveChoiceSelection = 0;
        pendingPassiveChoices = 0;
        passiveChoiceOptions = {GeneratedPassive{}, GeneratedPassive{}, GeneratedPassive{}};

        AddLog("Choose class: [1] Warrior  [2] Ranger  [3] Wizard");
    }

    void Update() {
        UpdateAudio();
        TickAfflictionFlashes();

        // Class selection gate
        if (classSelectionPending) {
            if (IsKeyPressed(KEY_ONE)) {
                PlaySfx(sfxInteract, sfxInteractLoaded);
                ApplyClassPreset(PlayerClass::Warrior);
            } else if (IsKeyPressed(KEY_TWO)) {
                PlaySfx(sfxInteract, sfxInteractLoaded);
                ApplyClassPreset(PlayerClass::Ranger);
            } else if (IsKeyPressed(KEY_THREE)) {
                PlaySfx(sfxInteract, sfxInteractLoaded);
                ApplyClassPreset(PlayerClass::Wizard);
            } else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                const int boxW = 260;
                const int boxH = 460;
                const int gap = 34;
                const int totalW = boxW * 3 + gap * 2;
                const int startX = (kScreenWidth - totalW) / 2;
                const int boxY = 160;

                Vector2 mousePos = GetMousePosition();
                for (int i = 0; i < 3; ++i) {
                    int x = startX + i * (boxW + gap);
                    Rectangle cardRect{static_cast<float>(x), static_cast<float>(boxY), static_cast<float>(boxW), static_cast<float>(boxH)};
                    if (!CheckCollisionPointRec(mousePos, cardRect)) continue;

                    PlaySfx(sfxInteract, sfxInteractLoaded);
                    if (i == 0) ApplyClassPreset(PlayerClass::Warrior);
                    if (i == 1) ApplyClassPreset(PlayerClass::Ranger);
                    if (i == 2) ApplyClassPreset(PlayerClass::Wizard);
                    break;
                }
            }
            return;
        }

        // Game-over gate (block all gameplay)
        if (gameOver) {
            if (IsKeyPressed(KEY_R)) {
                StartNewRun();
            }
            return;
        }

        if (IsKeyPressed(KEY_GRAVE)) {
            devConsoleOpen = !devConsoleOpen;
            if (devConsoleOpen) {
                devConsoleInput.clear();
                while (GetCharPressed() > 0) {}
            }
        }

        if (devConsoleOpen) {
            HandleDevConsoleInput();
            return;
        }

        if (passiveChoiceOpen) {
            HandlePassiveChoiceInput();
            return;
        }

        TryOpenMerchantRoom();

        if (merchantOpen) {
            HandleMerchantInput();
            return;
        }

        if (IsKeyPressed(KEY_E) && TryInteractWithNearbyActions()) {
            TickActiveSkillStateForTurn();
            CheckGameOver();
            if (!gameOver) {
                EnemyTurn();
                CheckGameOver();
            }
            return;
        }

        if (IsKeyPressed(KEY_E) && CanInteractWithMerchant()) {
            PlaySfx(sfxInteract, sfxInteractLoaded);
            merchantOpen = true;
            merchantTab = MerchantTab::Buy;
            merchantSelection = 0;
            return;
        }

        if (IsKeyPressed(KEY_I)) {
            inventoryOpen = !inventoryOpen;
            if (inventoryOpen) PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
            else PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            ClampInventorySelection();
        }

        if (inventoryOpen) {
            HandleInventoryInput();
            return;
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            pauseMenuOpen = !pauseMenuOpen;
            if (pauseMenuOpen) PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
            else PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            return;
        }

        if (pauseMenuOpen) {
            return;
        }

        if (IsKeyPressed(KEY_K)) {
            skillTreeOpen = !skillTreeOpen;
            if (skillTreeOpen) {
                PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
                skillSelection = std::clamp(skillSelection, 0, kClassSkillCount - 1);
            } else {
                PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            }
            return;
        }

        if (skillTreeOpen) {
            HandleSkillTreeInput();
            return;
        }

        int activeSkillSlot = GetPressedActiveSkillSlot();
        if (activeSkillSlot >= 0) {
            if (TryUseActiveSkillSlot(activeSkillSlot)) {
                TickActiveSkillStateForTurn();
                CheckGameOver();
                if (!gameOver) {
                    EnemyTurn();
                    CheckGameOver();
                }
            }
            return;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (TryMouseWeaponAttack()) {
                TickActiveSkillStateForTurn();
                CheckGameOver();
                if (!gameOver) {
                    EnemyTurn();
                    CheckGameOver();
                }
                return;
            }
        }

        bool consumedTurn = false;
        bool changedFloor = false;
        int oldPlayerX = player.x;
        int oldPlayerY = player.y;

        if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
            consumedTurn = PlayerTryMove(0, -1, changedFloor);
        } else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
            consumedTurn = PlayerTryMove(0, 1, changedFloor);
        } else if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
            consumedTurn = PlayerTryMove(-1, 0, changedFloor);
        } else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
            consumedTurn = PlayerTryMove(1, 0, changedFloor);
        }

        CheckGameOver();
        if (gameOver) return;

        if (consumedTurn && (player.x != oldPlayerX || player.y != oldPlayerY)) {
            player.staffCharge = std::min(5, player.staffCharge + 1);
        }

        if (consumedTurn) {
            TickActiveSkillStateForTurn();
            ApplyStepStatusEffects();
            CheckGameOver();
            if (gameOver) return;
        }

        if (consumedTurn && !changedFloor) {
            EnemyTurn();
            CheckGameOver();
        }
    }

    void Draw() const {
        // Important: don't draw map/entities before level exists
        if (classSelectionPending) {
            DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.85f));
            DrawText("Choose Your Class", kScreenWidth / 2 - 170, 78, 40, RAYWHITE);

            const int boxW = 260;
            const int boxH = 460;
            const int gap = 34;
            const int totalW = boxW * 3 + gap * 2;
            const int startX = (kScreenWidth - totalW) / 2;
            const int boxY = 160;

            struct ClassCard {
                PlayerClass cls;
                const char* keyLabel;
                const char* name;
                const char* weapon;
                int str;
                int dex;
                int intel;
                Color color;
            };

            const ClassCard cards[3] = {
                {PlayerClass::Warrior, "1", "Warrior", "Broken Sword", 10, 5, 3, Color{205, 70, 70, 255}},
                {PlayerClass::Ranger, "2", "Ranger", "Worn Bow", 5, 10, 3, Color{70, 175, 90, 255}},
                {PlayerClass::Wizard, "3", "Wizard", "Uncharged Staff", 3, 5, 10, Color{95, 140, 230, 255}},
            };

            for (int i = 0; i < 3; ++i) {
                int x = startX + i * (boxW + gap);
                Rectangle cardRect{static_cast<float>(x), static_cast<float>(boxY), static_cast<float>(boxW), static_cast<float>(boxH)};
                bool hovered = CheckCollisionPointRec(GetMousePosition(), cardRect);

                Color cardFill = hovered ? Color{34, 36, 52, 248} : Color{20, 20, 30, 242};
                Color borderColor = hovered
                    ? Color{
                        static_cast<unsigned char>(std::min(255, static_cast<int>(cards[i].color.r) + 30)),
                        static_cast<unsigned char>(std::min(255, static_cast<int>(cards[i].color.g) + 30)),
                        static_cast<unsigned char>(std::min(255, static_cast<int>(cards[i].color.b) + 30)),
                        255
                    }
                    : cards[i].color;

                DrawRectangleRec(cardRect, cardFill);
                DrawRectangleLinesEx(cardRect, hovered ? 4.0f : 3.0f, borderColor);

                DrawText(TextFormat("[%s]", cards[i].keyLabel), x + 16, boxY + 14, 28, LIGHTGRAY);

                Rectangle classIconSrc{};
                if (TryGetClassIconSource(cards[i].cls, classIconSrc)) {
                    const int iconSize = 124;
                    Rectangle dst{
                        static_cast<float>(x + (boxW - iconSize) / 2),
                        static_cast<float>(boxY + 58),
                        static_cast<float>(iconSize),
                        static_cast<float>(iconSize)
                    };
                    DrawTexturePro(playerAtlas, classIconSrc, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                } else {
                    DrawCircle(x + boxW / 2, boxY + 120, 50.0f, Fade(cards[i].color, 0.55f));
                }

                int classNameY = boxY + 200;
                int classNameW = MeasureText(cards[i].name, 34);
                DrawText(cards[i].name, x + (boxW - classNameW) / 2, classNameY, 34, RAYWHITE);

                DrawText("Starting Weapon", x + 20, boxY + 252, 22, GRAY);
                DrawText(cards[i].weapon, x + 20, boxY + 278, 24, LIGHTGRAY);

                DrawText("Initial Stats", x + 20, boxY + 326, 22, GRAY);
                DrawText(TextFormat("STR: %i", cards[i].str), x + 20, boxY + 352, 24, Color{255, 120, 120, 255});
                DrawText(TextFormat("DEX: %i", cards[i].dex), x + 20, boxY + 382, 24, Color{130, 230, 170, 255});
                DrawText(TextFormat("INT: %i", cards[i].intel), x + 20, boxY + 412, 24, Color{150, 190, 255, 255});
            }

            DrawText("Press 1, 2, or 3 to choose", kScreenWidth / 2 - 175, boxY + boxH + 22, 28, LIGHTGRAY);
            return;
        }

        const int mapViewW = kMapWidth * kTileSize;
        const int mapViewH = kMapHeight * kTileSize;
        Camera2D camera = BuildPlayerCamera();
        BeginScissorMode(0, 0, mapViewW, mapViewH);
        BeginMode2D(camera);
        DrawMap();
        DrawAttackPreview();
        DrawEnemyTelegraphs();
        DrawGoldStacks();
        DrawEntities();
        EndMode2D();
        EndScissorMode();
        DrawMapStatusOutline();
        DrawActiveSkillHotbar();
        DrawHud();

        if (devConsoleOpen) {
            DrawDevConsole();
        }

        if (inventoryOpen) {
            DrawInventoryOverlay();
        }

        if (merchantOpen) {
            DrawMerchantOverlay();
        }

        if (skillTreeOpen) {
            DrawSkillTreeOverlay();
        }

        if (passiveChoiceOpen) {
            DrawPassiveChoiceOverlay();
        }

        if (pauseMenuOpen) {
            DrawPauseMenu();
        }

        if (gameOver) {
            DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.65f));

            const char* title = "YOU HAVE DIED";
            const char* subtitle = "Press R to start a new run";
            const int titleSize = 60;
            const int subtitleSize = 24;

            const int titleW = MeasureText(title, titleSize);
            const int subtitleW = MeasureText(subtitle, subtitleSize);

            const int titleX = (kScreenWidth - titleW) / 2;
            const int subtitleX = (kScreenWidth - subtitleW) / 2;

            DrawText(title, titleX, kScreenHeight / 2 - 50, titleSize, RED);
            DrawText(subtitle, subtitleX, kScreenHeight / 2 + 20, subtitleSize, RAYWHITE);
        }

        DrawAfflictionFlashes();
    }

private:
    static constexpr int kEnemyViewRange = 8;
    static constexpr float kMapCameraZoom = 1.6f;
    static constexpr float kAfflictionFlashDuration = 0.16f;
    static constexpr int kClassSkillCount = 3;
    static constexpr int kClassSkillMaxRank = 10;

    std::vector<TileType> tiles;
    std::vector<Enemy> enemies;
    std::vector<GoldStack> goldStacks;
    std::vector<Decoration> walkableDecorations;
    std::vector<Decoration> solidDecorations;
    std::vector<Interactable> interactables;
    std::vector<Room> rooms;
    std::vector<Item> merchantStock;
    std::vector<std::string> combatLog;
    Player player;
    int dungeonLevel = 1;
    int nextMerchantFloor = 4;
    int score = 0;
    bool gameOver = false;
    bool merchantPresent = false;
    bool merchantOpen = false;
    int merchantX = -1;
    int merchantY = -1;
    int merchantDoorX = -1;
    int merchantDoorY = -1;
    bool merchantRoomOpened = false;
    int merchantIconVariant = 0;
    MerchantTab merchantTab = MerchantTab::Buy;
    int merchantSelection = 0;
    std::mt19937 rng{std::random_device{}()};
    bool classSelectionPending = false;
    bool inventoryOpen = false;
    int inventorySelection = 0;
    int inventoryScrollOffset = 0; // row offset for grid scrolling
    bool skillTreeOpen = false;
    int skillSelection = 0;
    bool passiveChoiceOpen = false;
    int passiveChoiceSelection = 0;
    int pendingPassiveChoices = 0;
    std::array<GeneratedPassive, 3> passiveChoiceOptions{};
    bool pauseMenuOpen = false;
    bool devConsoleOpen = false;
    std::string devConsoleInput;
    std::vector<std::string> devConsoleHistory;
    float poisonAfflictFlashTimer = 0.0f;
    float bleedAfflictFlashTimer = 0.0f;
    static constexpr int kInventoryGridCols = 8;
    static constexpr int kInventoryCellSize = 44;
    static constexpr int kInventoryCellGap = 6;
    static constexpr int kItemIconSize = 32;
    Texture2D roomAtlas{};
    bool roomAtlasLoaded = false;
    Texture2D actionAtlas{};
    bool actionAtlasLoaded = false;
    Texture2D walkablesAtlas{};
    bool walkablesAtlasLoaded = false;
    Texture2D solidsAtlas{};
    bool solidsAtlasLoaded = false;
    Texture2D stairsIcon{};
    bool stairsIconLoaded = false;
    Texture2D iconAtlas{};
    bool iconAtlasLoaded = false;
    Texture2D playerAtlas{};
    bool playerAtlasLoaded = false;
    Texture2D enemyAtlas{};
    bool enemyAtlasLoaded = false;
    Texture2D npcAtlas{};
    bool npcAtlasLoaded = false;
    Music musicBgm{};
    bool musicLoaded = false;
    Sound sfxAttack{};
    bool sfxAttackLoaded = false;
    Sound sfxAttack2{};
    bool sfxAttack2Loaded = false;
    Sound sfxAttack3{};
    bool sfxAttack3Loaded = false;
    Sound sfxAttackRanged{};
    bool sfxAttackRangedLoaded = false;
    Sound sfxAttackMagic{};
    bool sfxAttackMagicLoaded = false;
    Sound sfxEnemyDamaged{};
    bool sfxEnemyDamagedLoaded = false;
    Sound sfxEnemyDamaged2{};
    bool sfxEnemyDamaged2Loaded = false;
    Sound sfxEnemyDamaged3{};
    bool sfxEnemyDamaged3Loaded = false;
    Sound sfxHurt{};
    bool sfxHurtLoaded = false;
    Sound sfxHurt2{};
    bool sfxHurt2Loaded = false;
    Sound sfxHurt3{};
    bool sfxHurt3Loaded = false;
    Sound sfxBeastAttack{};
    bool sfxBeastAttackLoaded = false;
    Sound sfxBeastAttack2{};
    bool sfxBeastAttack2Loaded = false;
    Sound sfxGold{};
    bool sfxGoldLoaded = false;
    Sound sfxInteract{};
    bool sfxInteractLoaded = false;
    Sound sfxUseItem{};
    bool sfxUseItemLoaded = false;
    Sound sfxDecline{};
    bool sfxDeclineLoaded = false;
    Sound sfxMenuOpen{};
    bool sfxMenuOpenLoaded = false;
    Sound sfxMenuClose{};
    bool sfxMenuCloseLoaded = false;
    Sound sfxStairs{};
    bool sfxStairsLoaded = false;
    Sound sfxChestOpen{};
    bool sfxChestOpenLoaded = false;
    Sound sfxChestOpen2{};
    bool sfxChestOpen2Loaded = false;
    Sound sfxChestOpen3{};
    bool sfxChestOpen3Loaded = false;
    Sound sfxChestOpen4{};
    bool sfxChestOpen4Loaded = false;
    Sound sfxDeath{};
    bool sfxDeathLoaded = false;

    void LoadSfxIfExists(const char* path, Sound& sound, bool& loaded, float volume) {
        if (loaded) return;
        if (!FileExists(path)) return;
        sound = LoadSound(path);
        loaded = sound.frameCount > 0;
        if (loaded) SetSoundVolume(sound, volume);
    }

    void UnloadSfx(Sound& sound, bool& loaded) {
        if (!loaded) return;
        UnloadSound(sound);
        sound = Sound{};
        loaded = false;
    }

    void PlaySfx(Sound& sound, bool loaded, bool varyPitch = false, float pitchMin = 0.96f, float pitchMax = 1.04f) {
        if (!loaded) return;
        if (varyPitch) {
            std::uniform_real_distribution<float> pitchDist(pitchMin, pitchMax);
            SetSoundPitch(sound, pitchDist(rng));
        }
        PlaySound(sound);
        if (varyPitch) {
            SetSoundPitch(sound, 1.0f);
        }
    }

    void PlayRandomSfx(std::initializer_list<std::pair<Sound*, bool*>> candidates, bool varyPitch = false, float pitchMin = 0.96f, float pitchMax = 1.04f) {
        std::vector<Sound*> loadedSounds;
        loadedSounds.reserve(candidates.size());

        for (const auto& candidate : candidates) {
            Sound* sound = candidate.first;
            bool* loaded = candidate.second;
            if (sound == nullptr || loaded == nullptr) continue;
            if (!(*loaded)) continue;
            loadedSounds.push_back(sound);
        }

        if (loadedSounds.empty()) return;

        int pick = RandomInt(0, static_cast<int>(loadedSounds.size()) - 1);
        PlaySfx(*loadedSounds[pick], true, varyPitch, pitchMin, pitchMax);
    }

    void UpdateAudio() {
        if (!musicLoaded) return;
        UpdateMusicStream(musicBgm);
        if (!IsMusicStreamPlaying(musicBgm)) {
            PlayMusicStream(musicBgm);
        }
    }

    Camera2D BuildPlayerCamera() const {
        const float mapViewWidth = static_cast<float>(kMapWidth * kTileSize);
        const float mapViewHeight = static_cast<float>(kMapHeight * kTileSize);
        const float worldWidth = static_cast<float>(kMapWidth * kTileSize);
        const float worldHeight = static_cast<float>(kMapHeight * kTileSize);

        Camera2D camera{};
        camera.offset = Vector2{mapViewWidth * 0.5f, mapViewHeight * 0.5f};
        camera.rotation = 0.0f;
        camera.zoom = kMapCameraZoom;

        float targetX = static_cast<float>(player.x * kTileSize + kTileSize / 2);
        float targetY = static_cast<float>(player.y * kTileSize + kTileSize / 2);

        float halfViewWorldW = mapViewWidth * 0.5f / camera.zoom;
        float halfViewWorldH = mapViewHeight * 0.5f / camera.zoom;

        if (worldWidth > 2.0f * halfViewWorldW) {
            targetX = std::clamp(targetX, halfViewWorldW, worldWidth - halfViewWorldW);
        } else {
            targetX = worldWidth * 0.5f;
        }

        if (worldHeight > 2.0f * halfViewWorldH) {
            targetY = std::clamp(targetY, halfViewWorldH, worldHeight - halfViewWorldH);
        } else {
            targetY = worldHeight * 0.5f;
        }

        camera.target = Vector2{targetX, targetY};
        return camera;
    }

    bool TryGetMouseWorldTile(int& tileX, int& tileY) const {
        Vector2 mousePos = GetMousePosition();
        float mapW = static_cast<float>(kMapWidth * kTileSize);
        float mapH = static_cast<float>(kMapHeight * kTileSize);
        if (mousePos.x < 0 || mousePos.y < 0 || mousePos.x >= mapW || mousePos.y >= mapH) {
            return false;
        }

        Camera2D camera = BuildPlayerCamera();
        Vector2 world = GetScreenToWorld2D(mousePos, camera);
        tileX = static_cast<int>(std::floor(world.x / static_cast<float>(kTileSize)));
        tileY = static_cast<int>(std::floor(world.y / static_cast<float>(kTileSize)));
        if (!InBounds(tileX, tileY)) return false;
        return true;
    }

    void TriggerPoisonAfflictFlash() {
        poisonAfflictFlashTimer = kAfflictionFlashDuration;
    }

    void TriggerBleedAfflictFlash() {
        bleedAfflictFlashTimer = kAfflictionFlashDuration;
    }

    void TickAfflictionFlashes() {
        float dt = GetFrameTime();
        poisonAfflictFlashTimer = std::max(0.0f, poisonAfflictFlashTimer - dt);
        bleedAfflictFlashTimer = std::max(0.0f, bleedAfflictFlashTimer - dt);
    }

    void DrawAfflictionFlashes() const {
        if (poisonAfflictFlashTimer > 0.0f) {
            float t = poisonAfflictFlashTimer / kAfflictionFlashDuration;
            DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(Color{40, 220, 95, 255}, 0.28f * t));
        }
        if (bleedAfflictFlashTimer > 0.0f) {
            float t = bleedAfflictFlashTimer / kAfflictionFlashDuration;
            DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(Color{220, 35, 35, 255}, 0.28f * t));
        }
    }

    const char* SkillNameForClass(PlayerClass cls, int index) const {
        (void)cls;
        if (index == 0) return "Offense Path";
        if (index == 1) return "Balanced Path";
        return "Defense Path";
    }

    std::string SkillDescriptionForClass(PlayerClass cls, int index) const {
        (void)cls;
        if (index == 0) return "Path to your offense active skill (10 nodes)";
        if (index == 1) return "Path to your balanced active skill (10 nodes)";
        return "Path to your defense active skill (10 nodes)";
    }

    struct PassiveRuntimeEffects {
        float maxHpPct = 0.0f;
        float strengthPct = 0.0f;
        float dexterityPct = 0.0f;
        float intelligencePct = 0.0f;
        float armorPct = 0.0f;
        float damageMult = 1.0f;
        float goldDropMult = 1.0f;
        float expMult = 1.0f;
        float enemySpawnMult = 1.0f;

        float lifeLeechPct = 0.0f;
        float echoChance = 0.0f;
        float discoveryChance = 0.0f;
        float trickeryChance = 0.0f;
        float revivalChance = 0.0f;
        float contagionPct = 0.0f;
        float conduitPct = 0.0f;
        float charityPct = 0.0f;
        float miasmaPct = 0.0f;
        float radiancePct = 0.0f;
    };

    struct PassivePrefixEntry {
        const char* label;
        PassiveStatType statType;
        bool increases;
    };

    static constexpr std::array<PassivePrefixEntry, 18> kPassivePrefixTable{{
        {"Arabix'", PassiveStatType::MaxHp, true},
        {"Malphas'", PassiveStatType::MaxHp, false},
        {"Valerius'", PassiveStatType::Strength, true},
        {"Krell's", PassiveStatType::Strength, false},
        {"Sly's", PassiveStatType::Dexterity, true},
        {"Mordred's", PassiveStatType::Dexterity, false},
        {"Merlin's", PassiveStatType::Intelligence, true},
        {"Xerxes'", PassiveStatType::Intelligence, false},
        {"Iron-Clad's", PassiveStatType::Armor, true},
        {"Void's", PassiveStatType::Armor, false},
        {"Astraea's", PassiveStatType::Damage, true},
        {"Blight's", PassiveStatType::Damage, false},
        {"Midas'", PassiveStatType::GoldDropped, true},
        {"Gelt's", PassiveStatType::GoldDropped, false},
        {"Sage's", PassiveStatType::ExpEarned, true},
        {"Lethe's", PassiveStatType::ExpEarned, false},
        {"Solari's", PassiveStatType::EnemySpawned, false},
        {"Bane's", PassiveStatType::EnemySpawned, true}
    }};

    const char* PassiveSuffixName(PassiveSuffixId suffixId) const {
        switch (suffixId) {
        case PassiveSuffixId::Vampirism: return "Vampirism";
        case PassiveSuffixId::Larceny: return "Larceny";
        case PassiveSuffixId::Punishment: return "Punishment";
        case PassiveSuffixId::Fortress: return "Fortress";
        case PassiveSuffixId::Echo: return "Echo";
        case PassiveSuffixId::Discovery: return "Discovery";
        case PassiveSuffixId::Enlightenment: return "Enlightenment";
        case PassiveSuffixId::Trickery: return "Trickery";
        case PassiveSuffixId::Revival: return "Revival";
        case PassiveSuffixId::Contagion: return "Contagion";
        case PassiveSuffixId::Conduit: return "Conduit";
        case PassiveSuffixId::Charity: return "Charity";
        case PassiveSuffixId::Miasma: return "Miasma";
        case PassiveSuffixId::Radiance: return "Radiance";
        default: return "Passive";
        }
    }

    const char* PassiveGradeName(ItemTier tier) const {
        switch (tier) {
        case ItemTier::Common: return "Weak";
        case ItemTier::Uncommon: return "Minor";
        case ItemTier::Rare: return "Lesser";
        case ItemTier::Mythic: return "Greater";
        case ItemTier::Legendary: return "Grand";
        default: return "Weak";
        }
    }

    std::string PassivePrefixName(PassiveStatType statType, bool increases) const {
        for (const PassivePrefixEntry& entry : kPassivePrefixTable) {
            if (entry.statType == statType && entry.increases == increases) {
                return entry.label;
            }
        }
        return increases ? "Heroic" : "Villainous";
    }

    std::string PassiveSkillName(const GeneratedPassive& passive) const {
        return PassivePrefixName(passive.statType, passive.increasesStat) + " " + PassiveGradeName(passive.grade) + " " + PassiveSuffixName(passive.suffix);
    }

    std::string PassiveSkillDescription(const GeneratedPassive& passive) const {
        int statPct = static_cast<int>(std::round(passive.statPercent * 100.0f));
        int suffixPct = static_cast<int>(std::round(passive.passivePercent * 100.0f));
        std::string prefixEffect = std::string(passive.increasesStat ? "+" : "-") + std::to_string(statPct) + "% ";
        switch (passive.statType) {
        case PassiveStatType::MaxHp: prefixEffect += "Max HP"; break;
        case PassiveStatType::Strength: prefixEffect += "Strength"; break;
        case PassiveStatType::Dexterity: prefixEffect += "Dexterity"; break;
        case PassiveStatType::Intelligence: prefixEffect += "Intelligence"; break;
        case PassiveStatType::Armor: prefixEffect += "Armor"; break;
        case PassiveStatType::Damage: prefixEffect += "Damage"; break;
        case PassiveStatType::GoldDropped: prefixEffect += "Gold Dropped"; break;
        case PassiveStatType::ExpEarned: prefixEffect += "EXP Earned"; break;
        case PassiveStatType::EnemySpawned: prefixEffect += "Enemies Spawned"; break;
        }

        return prefixEffect + " | " + PassiveSuffixName(passive.suffix) + " +" + std::to_string(suffixPct) + "%";
    }

    PassiveRuntimeEffects ComputePassiveEffects() const {
        PassiveRuntimeEffects effects;

        for (const GeneratedPassive& passive : player.passiveSkills) {
            float signedStatPct = passive.increasesStat ? passive.statPercent : -passive.statPercent;

            switch (passive.statType) {
            case PassiveStatType::MaxHp: effects.maxHpPct += signedStatPct; break;
            case PassiveStatType::Strength: effects.strengthPct += signedStatPct; break;
            case PassiveStatType::Dexterity: effects.dexterityPct += signedStatPct; break;
            case PassiveStatType::Intelligence: effects.intelligencePct += signedStatPct; break;
            case PassiveStatType::Armor: effects.armorPct += signedStatPct; break;
            case PassiveStatType::Damage: effects.damageMult *= std::max(0.10f, 1.0f + signedStatPct); break;
            case PassiveStatType::GoldDropped: effects.goldDropMult *= std::max(0.10f, 1.0f + signedStatPct); break;
            case PassiveStatType::ExpEarned: effects.expMult *= std::max(0.10f, 1.0f + signedStatPct); break;
            case PassiveStatType::EnemySpawned: effects.enemySpawnMult *= std::max(0.20f, 1.0f + signedStatPct); break;
            }

            switch (passive.suffix) {
            case PassiveSuffixId::Vampirism: effects.lifeLeechPct += passive.passivePercent; break;
            case PassiveSuffixId::Larceny: effects.goldDropMult *= (1.0f + passive.passivePercent); break;
            case PassiveSuffixId::Punishment: effects.damageMult *= (1.0f + passive.passivePercent); break;
            case PassiveSuffixId::Fortress: effects.armorPct += passive.passivePercent; break;
            case PassiveSuffixId::Echo: effects.echoChance += passive.passivePercent; break;
            case PassiveSuffixId::Discovery: effects.discoveryChance += passive.passivePercent; break;
            case PassiveSuffixId::Enlightenment: effects.expMult *= (1.0f + passive.passivePercent); break;
            case PassiveSuffixId::Trickery: effects.trickeryChance += passive.passivePercent; break;
            case PassiveSuffixId::Revival: effects.revivalChance += passive.passivePercent; break;
            case PassiveSuffixId::Contagion: effects.contagionPct += passive.passivePercent; break;
            case PassiveSuffixId::Conduit: effects.conduitPct += passive.passivePercent; break;
            case PassiveSuffixId::Charity: effects.charityPct += passive.passivePercent; break;
            case PassiveSuffixId::Miasma: effects.miasmaPct += passive.passivePercent; break;
            case PassiveSuffixId::Radiance: effects.radiancePct += passive.passivePercent; break;
            }
        }

        effects.echoChance = std::clamp(effects.echoChance, 0.0f, 0.95f);
        effects.discoveryChance = std::clamp(effects.discoveryChance, 0.0f, 0.95f);
        effects.trickeryChance = std::clamp(effects.trickeryChance, 0.0f, 0.95f);
        effects.revivalChance = std::clamp(effects.revivalChance, 0.0f, 0.98f);
        return effects;
    }

    GeneratedPassive GeneratePassiveSkill() {
        GeneratedPassive passive;

        int prefixIndex = RandomInt(0, static_cast<int>(kPassivePrefixTable.size()) - 1);
        const PassivePrefixEntry& prefix = kPassivePrefixTable[static_cast<size_t>(prefixIndex)];
        passive.statType = prefix.statType;
        passive.increasesStat = prefix.increases;
        passive.grade = RollItemTier();
        passive.suffix = static_cast<PassiveSuffixId>(RandomInt(0, 13));

        static const float statPctByTier[5] = {0.02f, 0.05f, 0.10f, 0.15f, 0.25f};
        static const float heroicSuffixPctByTier[5] = {0.03f, 0.07f, 0.12f, 0.20f, 0.35f};
        static const float villainSuffixPctByTier[5] = {0.06f, 0.14f, 0.24f, 0.40f, 0.70f};

        int tierIndex = static_cast<int>(passive.grade);
        if (tierIndex < 0) tierIndex = 0;
        if (tierIndex > 4) tierIndex = 4;

        passive.statPercent = statPctByTier[tierIndex];
        passive.passivePercent = passive.increasesStat ? heroicSuffixPctByTier[tierIndex] : villainSuffixPctByTier[tierIndex];
        return passive;
    }

    void GetSkillBonuses(int& bonusMaxHp, int& bonusArmor, int& bonusStrength, int& bonusDexterity, int& bonusIntelligence, int& bonusAttack) const {
        bonusMaxHp = 0;
        bonusArmor = 0;
        bonusStrength = 0;
        bonusDexterity = 0;
        bonusIntelligence = 0;
        bonusAttack = 0;
    }

    bool TrySpendSkillPoint(int index) {
        if (index < 0 || index >= kClassSkillCount) return false;
        if (player.skillPoints <= 0) return false;
        if (player.classSkillRanks[index] >= kClassSkillMaxRank) return false;

        player.classSkillRanks[index] += 1;
        player.skillPoints -= 1;
        RecomputePlayerStats();
        AddLog(std::string("Upgraded ") + SkillNameForClass(player.playerClass, index) + ".");
        if (player.classSkillRanks[index] >= kClassSkillMaxRank) {
            ActiveSkillId unlockedSkill = player.activeSkillLoadout[static_cast<size_t>(index)];
            if (unlockedSkill != ActiveSkillId::None) {
                AddLog(std::string("Active unlocked: ") + ActiveSkillName(unlockedSkill) + " (" + std::to_string(index + 1) + ").");
            }
        }
        return true;
    }

    void GeneratePassiveChoiceOffer() {
        std::array<GeneratedPassive, 3> options{};
        int filled = 0;
        while (filled < 3) {
            GeneratedPassive candidate = GeneratePassiveSkill();
            std::string candidateName = PassiveSkillName(candidate);
            bool exists = false;
            for (int i = 0; i < filled; ++i) {
                if (PassiveSkillName(options[i]) == candidateName) {
                    exists = true;
                    break;
                }
            }
            if (exists) continue;
            options[filled] = candidate;
            filled += 1;
        }

        passiveChoiceOptions = options;
        passiveChoiceSelection = 0;
        passiveChoiceOpen = true;
        AddLog("Passive draft ready: choose 1 of 3.");
    }

    void ApplyPassiveChoice(int optionIndex) {
        if (optionIndex < 0 || optionIndex >= 3) return;
        const GeneratedPassive& chosen = passiveChoiceOptions[static_cast<size_t>(optionIndex)];
        player.passiveSkills.push_back(chosen);
        RecomputePlayerStats();
        AddLog(std::string("Passive gained: ") + PassiveSkillName(chosen) + ".");

        if (pendingPassiveChoices > 0) {
            pendingPassiveChoices -= 1;
            GeneratePassiveChoiceOffer();
        } else {
            passiveChoiceOpen = false;
        }
    }

    const char* ActiveSkillName(ActiveSkillId skillId) const {
        switch (skillId) {
        case ActiveSkillId::WarriorCleave: return "Cleave";
        case ActiveSkillId::WarriorSecondWind: return "Second Wind";
        case ActiveSkillId::WarriorShieldWall: return "Shield Wall";
        case ActiveSkillId::RangerPiercingShot: return "Piercing Shot";
        case ActiveSkillId::RangerVolley: return "Volley";
        case ActiveSkillId::RangerSurvivalInstinct: return "Survival Instinct";
        case ActiveSkillId::WizardArcaneNova: return "Arcane Nova";
        case ActiveSkillId::WizardBlink: return "Blink";
        case ActiveSkillId::WizardBarrier: return "Arcane Barrier";
        default: return "Empty";
        }
    }

    int ActiveSkillPassiveIndex(ActiveSkillId skillId) const {
        switch (skillId) {
        case ActiveSkillId::WarriorCleave:
        case ActiveSkillId::RangerPiercingShot:
        case ActiveSkillId::WizardArcaneNova:
            return 0;
        case ActiveSkillId::WarriorSecondWind:
        case ActiveSkillId::RangerVolley:
        case ActiveSkillId::WizardBlink:
            return 1;
        case ActiveSkillId::WarriorShieldWall:
        case ActiveSkillId::RangerSurvivalInstinct:
        case ActiveSkillId::WizardBarrier:
            return 2;
        default:
            return -1;
        }
    }

    int ActiveSkillBaseCooldown(ActiveSkillId skillId) const {
        switch (skillId) {
        case ActiveSkillId::WarriorCleave: return 4;
        case ActiveSkillId::WarriorSecondWind: return 7;
        case ActiveSkillId::WarriorShieldWall: return 8;
        case ActiveSkillId::RangerPiercingShot: return 4;
        case ActiveSkillId::RangerVolley: return 6;
        case ActiveSkillId::RangerSurvivalInstinct: return 8;
        case ActiveSkillId::WizardArcaneNova: return 5;
        case ActiveSkillId::WizardBlink: return 6;
        case ActiveSkillId::WizardBarrier: return 8;
        default: return 0;
        }
    }

    int GetPressedActiveSkillSlot() const {
        if (IsKeyPressed(KEY_ONE)) return 0;
        if (IsKeyPressed(KEY_TWO)) return 1;
        if (IsKeyPressed(KEY_THREE)) return 2;
        if (IsKeyPressed(KEY_FOUR)) return 3;
        if (IsKeyPressed(KEY_FIVE)) return 4;
        if (IsKeyPressed(KEY_SIX)) return 5;
        if (IsKeyPressed(KEY_SEVEN)) return 6;
        if (IsKeyPressed(KEY_EIGHT)) return 7;
        if (IsKeyPressed(KEY_NINE)) return 8;
        if (IsKeyPressed(KEY_ZERO)) return 9;
        return -1;
    }

    void TriggerCharityBuffIfApplicable() {
        PassiveRuntimeEffects effects = ComputePassiveEffects();
        if (effects.charityPct <= 0.0f) return;

        int attackBoost = std::max(1, static_cast<int>(std::round(static_cast<float>(player.atk) * effects.charityPct * 0.20f)));
        int armorBoost = std::max(1, static_cast<int>(std::round(2.0f + effects.charityPct * 6.0f)));
        player.temporaryAttackBonus += attackBoost;
        player.temporaryAttackTurns = std::max(player.temporaryAttackTurns, 3);
        player.temporaryArmorBonus += armorBoost;
        player.temporaryArmorTurns = std::max(player.temporaryArmorTurns, 3);
        RecomputePlayerStats();
        AddLog("Charity empowers you for 3 turns.");
    }

    void ApplyPassiveAurasOnTurn() {
        PassiveRuntimeEffects effects = ComputePassiveEffects();
        if (effects.miasmaPct <= 0.0f && effects.radiancePct <= 0.0f) return;

        int radius = 1;
        radius = std::max(radius, std::min(3, 1 + static_cast<int>(std::floor(effects.miasmaPct * 4.0f))));
        radius = std::max(radius, std::min(3, 1 + static_cast<int>(std::floor(effects.radiancePct * 4.0f))));

        bool miasmaHit = false;
        bool radianceHit = false;
        for (int i = 0; i < static_cast<int>(enemies.size()); ++i) {
            Enemy& enemy = enemies[i];
            if (!enemy.alive) continue;

            int distance = std::abs(enemy.x - player.x) + std::abs(enemy.y - player.y);
            if (distance > radius) continue;

            if (effects.miasmaPct > 0.0f) {
                int damage = std::max(1, static_cast<int>(std::round(static_cast<float>(player.atk) * effects.miasmaPct * 0.18f)));
                DamageEnemyByPlayer(i, damage);
                miasmaHit = true;
            }

            if (!enemy.alive) continue;

            if (effects.radiancePct > 0.0f) {
                int damage = std::max(1, static_cast<int>(std::round(static_cast<float>(player.atk) * effects.radiancePct * 0.24f)));
                DamageEnemyByPlayer(i, damage);
                radianceHit = true;
            }
        }

        if (miasmaHit) AddLog("Miasma harms nearby enemies.");
        if (radianceHit) AddLog("Radiance burns nearby enemies.");
    }

    void TickActiveSkillStateForTurn() {
        ApplyPassiveAurasOnTurn();

        for (int& cooldown : player.activeSkillCooldowns) {
            cooldown = std::max(0, cooldown - 1);
        }

        if (player.temporaryArmorTurns > 0) {
            player.temporaryArmorTurns -= 1;
            if (player.temporaryArmorTurns <= 0) {
                player.temporaryArmorTurns = 0;
                player.temporaryArmorBonus = 0;
                RecomputePlayerStats();
                AddLog("Your defensive buff fades.");
            }
        }

        if (player.temporaryAttackTurns > 0) {
            player.temporaryAttackTurns -= 1;
            if (player.temporaryAttackTurns <= 0) {
                player.temporaryAttackTurns = 0;
                player.temporaryAttackBonus = 0;
                RecomputePlayerStats();
                AddLog("Your offensive buff fades.");
            }
        }
    }

    bool TryUseActiveSkillSlot(int slotIndex, bool allowEcho = true) {
        if (slotIndex < 0 || slotIndex >= static_cast<int>(player.activeSkillLoadout.size())) return false;

        ActiveSkillId skillId = player.activeSkillLoadout[slotIndex];
        if (skillId == ActiveSkillId::None) {
            AddLog("No skill bound to that slot.");
            return false;
        }

        int requiredPassiveIndex = ActiveSkillPassiveIndex(skillId);
        if (requiredPassiveIndex >= 0 && player.classSkillRanks[requiredPassiveIndex] < kClassSkillMaxRank) {
            AddLog(std::string("Complete ") + SkillNameForClass(player.playerClass, requiredPassiveIndex) + " to rank " + std::to_string(kClassSkillMaxRank) + ".");
            return false;
        }

        if (player.activeSkillCooldowns[slotIndex] > 0) {
            AddLog(std::string(ActiveSkillName(skillId)) + " cooldown: " + std::to_string(player.activeSkillCooldowns[slotIndex]) + " turn(s).");
            return false;
        }

        bool used = false;

        if (skillId == ActiveSkillId::WarriorCleave) {
            int hits = 0;
            for (int enemyIndex = 0; enemyIndex < static_cast<int>(enemies.size()); ++enemyIndex) {
                Enemy& enemy = enemies[enemyIndex];
                if (!enemy.alive) continue;
                int dx = std::abs(enemy.x - player.x);
                int dy = std::abs(enemy.y - player.y);
                if (dx <= 1 && dy <= 1 && (dx + dy) > 0) {
                    DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk + 2));
                    hits += 1;
                }
            }
            if (hits > 0) {
                AddLog("Cleave hits " + std::to_string(hits) + " target(s).");
                used = true;
            } else {
                AddLog("No enemies adjacent for Cleave.");
            }
        } else if (skillId == ActiveSkillId::WarriorSecondWind) {
            int heal = std::max(6, player.maxHp / 4);
            int oldHp = player.hp;
            player.hp = std::min(player.maxHp, player.hp + heal);
            AddLog("Second Wind restores " + std::to_string(player.hp - oldHp) + " HP.");
            used = true;
        } else if (skillId == ActiveSkillId::WarriorShieldWall) {
            player.temporaryArmorBonus = 4;
            player.temporaryArmorTurns = 3;
            RecomputePlayerStats();
            AddLog("Shield Wall: +4 armor for 3 turns.");
            used = true;
        } else if (skillId == ActiveSkillId::RangerPiercingShot) {
            int targetX = 0;
            int targetY = 0;
            if (TryGetMouseWorldTile(targetX, targetY) && AttackBowRay(targetX, targetY)) {
                AddLog("Piercing Shot fired.");
                used = true;
            } else {
                AddLog("No valid shot line.");
            }
        } else if (skillId == ActiveSkillId::RangerVolley) {
            int shots = 0;
            for (int enemyIndex = 0; enemyIndex < static_cast<int>(enemies.size()); ++enemyIndex) {
                Enemy& enemy = enemies[enemyIndex];
                if (!enemy.alive) continue;
                if (!HasLineOfSight(player.x, player.y, enemy.x, enemy.y)) continue;
                DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk));
                shots += 1;
                if (shots >= 3) break;
            }
            if (shots > 0) {
                AddLog("Volley strikes " + std::to_string(shots) + " target(s).");
                used = true;
            } else {
                AddLog("No targets in sight for Volley.");
            }
        } else if (skillId == ActiveSkillId::RangerSurvivalInstinct) {
            bool cured = player.poisoned || player.bleeding;
            player.poisoned = false;
            player.bleeding = false;
            int heal = std::max(4, player.maxHp / 8);
            int oldHp = player.hp;
            player.hp = std::min(player.maxHp, player.hp + heal);
            AddLog(std::string("Survival Instinct: +") + std::to_string(player.hp - oldHp) + " HP" + (cured ? ", ailments cured." : "."));
            used = true;
        } else if (skillId == ActiveSkillId::WizardArcaneNova) {
            int hits = 0;
            for (int enemyIndex = 0; enemyIndex < static_cast<int>(enemies.size()); ++enemyIndex) {
                Enemy& enemy = enemies[enemyIndex];
                if (!enemy.alive) continue;
                int md = std::abs(enemy.x - player.x) + std::abs(enemy.y - player.y);
                if (md <= 2) {
                    DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk + 1));
                    hits += 1;
                }
            }
            if (hits > 0) {
                AddLog("Arcane Nova hits " + std::to_string(hits) + " target(s).");
                used = true;
            } else {
                AddLog("No enemies in Arcane Nova range.");
            }
        } else if (skillId == ActiveSkillId::WizardBlink) {
            int targetX = 0;
            int targetY = 0;
            if (TryGetMouseWorldTile(targetX, targetY) && IsWalkable(targetX, targetY) && !IsEnemyAt(targetX, targetY)) {
                int dist = std::abs(targetX - player.x) + std::abs(targetY - player.y);
                if (dist <= 6) {
                    player.x = targetX;
                    player.y = targetY;
                    AddLog("Blink.");
                    used = true;
                } else {
                    AddLog("Blink target is too far.");
                }
            } else {
                AddLog("Invalid Blink target.");
            }
        } else if (skillId == ActiveSkillId::WizardBarrier) {
            player.temporaryArmorBonus = 5;
            player.temporaryArmorTurns = 3;
            RecomputePlayerStats();
            AddLog("Arcane Barrier: +5 armor for 3 turns.");
            used = true;
        }

        if (!used) return false;

        PassiveRuntimeEffects passiveEffects = ComputePassiveEffects();
        int cooldown = ActiveSkillBaseCooldown(skillId);
        bool skipCooldown = passiveEffects.trickeryChance > 0.0f && RollChance(passiveEffects.trickeryChance);
        if (skipCooldown) {
            player.activeSkillCooldowns[slotIndex] = 0;
            AddLog("Trickery skips cooldown!");
        } else {
            player.activeSkillCooldowns[slotIndex] = std::max(0, cooldown + 1);
        }

        if (allowEcho && passiveEffects.echoChance > 0.0f && RollChance(passiveEffects.echoChance)) {
            AddLog("Echo repeats the skill!");
            player.activeSkillCooldowns[slotIndex] = 0;
            TryUseActiveSkillSlot(slotIndex, false);
        }
        return true;
    }

    const char* EquipSlotLabel(EquipSlot slot) const {
        switch (slot) {
        case EquipSlot::Head: return "Head";
        case EquipSlot::Top: return "Top";
        case EquipSlot::Feet: return "Feet";
        case EquipSlot::Hands: return "Hands";
        case EquipSlot::Ring1: return "Ring 1";
        case EquipSlot::Ring2: return "Ring 2";
        case EquipSlot::Necklace: return "Necklace";
        case EquipSlot::Weapon: return "Weapon";
        case EquipSlot::Shield: return "Shield";
        default: return "Unknown";
        }
    }

    const char* MaterialPrefixForTier(ItemTier tier) const {
        switch (tier) {
        case ItemTier::Common: return "Bronze";
        case ItemTier::Uncommon: return "Iron";
        case ItemTier::Rare: return "Steel";
        case ItemTier::Mythic: return "Mithril";
        case ItemTier::Legendary: return "Vorpal";
        default: return "Bronze";
        }
    }

    const char* BaseItemNameForSlot(EquipSlot slot) const {
        switch (slot) {
        case EquipSlot::Head: return "Helm";
        case EquipSlot::Top: return "Armor";
        case EquipSlot::Feet: return "Boots";
        case EquipSlot::Hands: return "Gauntlets";
        case EquipSlot::Ring1:
        case EquipSlot::Ring2:
            return "Ring";
        case EquipSlot::Necklace: return "Amulet";
        case EquipSlot::Weapon: return "Sword";
        case EquipSlot::Shield: return "Shield";
        default: return "Relic";
        }
    }

    const char* ItemGlyph(const Item& item) const {
        if (item.category == ItemCategory::Consumable) {
            switch (item.consumableType) {
            case ConsumableType::HealthPotion: return "H";
            case ConsumableType::ArmorPotion: return "A";
            case ConsumableType::Antidote: return "T";
            case ConsumableType::Bandage: return "B";
            default: return "!";
            }
        }

        if (item.slot == EquipSlot::Weapon) {
            if (item.weaponType == WeaponType::Sword) return "S";
            if (item.weaponType == WeaponType::Bow) return "W";
            return "F";
        }
        if (item.slot == EquipSlot::Shield) return "D";
        if (item.slot == EquipSlot::Head) return "H";
        if (item.slot == EquipSlot::Top) return "C";
        if (item.slot == EquipSlot::Feet) return "U";
        if (item.slot == EquipSlot::Hands) return "G";
        if (item.slot == EquipSlot::Necklace) return "N";
        if (item.slot == EquipSlot::Ring1 || item.slot == EquipSlot::Ring2) return "R";
        return "?";
    }

    bool TryGetClassIconSource(PlayerClass cls, Rectangle& source) const {
        if (!playerAtlasLoaded) return false;

        int atlasCol = 0;
        if (cls == PlayerClass::Warrior) atlasCol = 0;
        else if (cls == PlayerClass::Ranger) atlasCol = 1;
        else atlasCol = 2;

        float sx = static_cast<float>(atlasCol * kItemIconSize);
        float sy = 0.0f;
        source = Rectangle{sx, sy, static_cast<float>(kItemIconSize), static_cast<float>(kItemIconSize)};

        if ((sx + kItemIconSize) > static_cast<float>(playerAtlas.width) ||
            (sy + kItemIconSize) > static_cast<float>(playerAtlas.height)) {
            return false;
        }

        return true;
    }

    bool TryGetEnemyIconSource(int variant, Rectangle& source) const {
        if (!enemyAtlasLoaded) return false;

        int cols = enemyAtlas.width / kItemIconSize;
        int rows = enemyAtlas.height / kItemIconSize;
        if (cols <= 0 || rows <= 0) return false;

        int tileCount = cols * rows;
        int clampedVariant = (tileCount > 0) ? (variant % tileCount) : 0;
        if (clampedVariant < 0) clampedVariant = 0;

        int col = clampedVariant % cols;
        int row = clampedVariant / cols;

        float sx = static_cast<float>(col * kItemIconSize);
        float sy = static_cast<float>(row * kItemIconSize);
        source = Rectangle{sx, sy, static_cast<float>(kItemIconSize), static_cast<float>(kItemIconSize)};
        return true;
    }

    bool TryGetNpcIconSource(int variant, Rectangle& source) const {
        if (!npcAtlasLoaded) return false;

        int cols = npcAtlas.width / kItemIconSize;
        int rows = npcAtlas.height / kItemIconSize;
        if (cols <= 0 || rows <= 0) return false;

        int tileCount = cols * rows;
        int clampedVariant = (tileCount > 0) ? (variant % tileCount) : 0;
        if (clampedVariant < 0) clampedVariant = 0;

        int col = clampedVariant % cols;
        int row = clampedVariant / cols;

        float sx = static_cast<float>(col * kItemIconSize);
        float sy = static_cast<float>(row * kItemIconSize);
        source = Rectangle{sx, sy, static_cast<float>(kItemIconSize), static_cast<float>(kItemIconSize)};
        return true;
    }

    bool TryGetActionIconSource(int stateCol, int stateRow, Rectangle& source) const {
        if (!actionAtlasLoaded) return false;

        int cols = actionAtlas.width / kItemIconSize;
        int rows = actionAtlas.height / kItemIconSize;
        if (cols <= 0 || rows <= 0) return false;

        int clampedCol = std::clamp(stateCol, 0, cols - 1);
        int clampedRow = std::clamp(stateRow, 0, rows - 1);
        source = Rectangle{
            static_cast<float>(clampedCol * kItemIconSize),
            static_cast<float>(clampedRow * kItemIconSize),
            static_cast<float>(kItemIconSize),
            static_cast<float>(kItemIconSize)
        };
        return true;
    }

    bool UseWoodDoorTheme() const {
        return dungeonLevel <= 10 || (dungeonLevel >= 41 && dungeonLevel <= 50);
    }

    bool TryGetWalkableDecorationSource(int variant, Rectangle& source) const {
        if (!walkablesAtlasLoaded) return false;

        int cols = walkablesAtlas.width / kItemIconSize;
        int rows = walkablesAtlas.height / kItemIconSize;
        if (cols <= 0 || rows <= 0) return false;

        int tileCount = cols * rows;
        int clampedVariant = (tileCount > 0) ? (variant % tileCount) : 0;
        if (clampedVariant < 0) clampedVariant = 0;

        int col = clampedVariant % cols;
        int row = clampedVariant / cols;
        source = Rectangle{
            static_cast<float>(col * kItemIconSize),
            static_cast<float>(row * kItemIconSize),
            static_cast<float>(kItemIconSize),
            static_cast<float>(kItemIconSize)
        };
        return true;
    }

    bool TryGetSolidDecorationSource(int variant, Rectangle& source) const {
        if (!solidsAtlasLoaded) return false;

        int cols = solidsAtlas.width / kItemIconSize;
        int rows = solidsAtlas.height / kItemIconSize;
        if (cols <= 0 || rows <= 0) return false;

        int tileCount = cols * rows;
        int clampedVariant = (tileCount > 0) ? (variant % tileCount) : 0;
        if (clampedVariant < 0) clampedVariant = 0;

        int col = clampedVariant % cols;
        int row = clampedVariant / cols;
        source = Rectangle{
            static_cast<float>(col * kItemIconSize),
            static_cast<float>(row * kItemIconSize),
            static_cast<float>(kItemIconSize),
            static_cast<float>(kItemIconSize)
        };
        return true;
    }

    struct RoomThemeRows {
        int wallRow = 0;
        int floorRow = 1;
        int outsideRow = 2;
        int wallCols = 1;
        int floorWallStartCol = 1;
        int floorWallCols = 1;
        int floorCols = 4;
        int outsideCols = 4;
    };

    RoomThemeRows CurrentRoomThemeRows() const {
        if (dungeonLevel <= 10) return RoomThemeRows{0, 1, 2, 1, 1, 1, 4, 4};
        if (dungeonLevel <= 20) return RoomThemeRows{3, 4, 5, 1, 1, 1, 4, 4};
        if (dungeonLevel <= 30) return RoomThemeRows{6, 7, 8, 1, 1, 2, 4, 4};
        if (dungeonLevel <= 40) return RoomThemeRows{9, 10, 11, 1, 1, 1, 4, 4};
        if (dungeonLevel <= 50) return RoomThemeRows{12, 13, 14, 1, 1, 2, 4, 4};
        return RoomThemeRows{15, 16, 17, 1, 1, 1, 4, 4};
    }

    bool TryGetRoomTileSource(TileType tileType, int tileX, int tileY, Rectangle& source) const {
        if (!roomAtlasLoaded) return false;

        int cols = roomAtlas.width / kItemIconSize;
        int rows = roomAtlas.height / kItemIconSize;
        if (cols <= 0 || rows <= 0) return false;

        RoomThemeRows theme = CurrentRoomThemeRows();

        int hash = (tileX * 73856093) ^ (tileY * 19349663);
        if (hash < 0) hash = -hash;

        auto weightedVariantCol = [&](int weightedHash, int availableCols) {
            if (availableCols <= 1) return 0;
            int roll = weightedHash % 100;
            if (roll < 80) return 0;
            if (roll < 90) return (availableCols > 1) ? 1 : 0;
            if (roll < 97) return (availableCols > 2) ? 2 : 0;
            return (availableCols > 3) ? 3 : 0;
        };

        int atlasRow = theme.outsideRow;
        int atlasCol = 0;

        if (tileType == TileType::Floor || tileType == TileType::Stairs) {
            atlasRow = theme.floorRow;
            int safeFloorCols = std::max(1, std::min(theme.floorCols, cols));
            atlasCol = weightedVariantCol(hash, safeFloorCols);
        } else if (tileType == TileType::Wall) {
            bool hasFloorBelow = false;
            if (tileY + 1 < kMapHeight) {
                TileType below = tiles[Index(tileX, tileY + 1)];
                hasFloorBelow = (below == TileType::Floor || below == TileType::Stairs);
            }

            atlasRow = theme.wallRow;
            if (hasFloorBelow && theme.floorWallCols > 0) {
                int floorWallStart = std::clamp(theme.floorWallStartCol, 0, std::max(0, cols - 1));
                int maxUsable = std::max(0, cols - floorWallStart);
                int safeFloorWallCols = std::max(1, std::min(theme.floorWallCols, maxUsable));
                atlasCol = floorWallStart + (hash % safeFloorWallCols);
            } else {
                int safeWallCols = std::max(1, std::min(theme.wallCols, cols));
                atlasCol = hash % safeWallCols;
            }
        } else {
            int safeOutsideCols = std::max(1, std::min(theme.outsideCols, cols));
            atlasRow = theme.outsideRow;
            atlasCol = weightedVariantCol(hash ^ 0x4F1BBCDC, safeOutsideCols);
        }

        if (atlasRow < 0 || atlasRow >= rows) return false;
        if (atlasCol < 0 || atlasCol >= cols) return false;

        float sx = static_cast<float>(atlasCol * kItemIconSize);
        float sy = static_cast<float>(atlasRow * kItemIconSize);
        source = Rectangle{sx, sy, static_cast<float>(kItemIconSize), static_cast<float>(kItemIconSize)};
        return true;
    }

    bool TryGetGoldIconSource(int iconCol, Rectangle& source) const {
        if (!iconAtlasLoaded) return false;

        const int atlasRow = 11;
        float sx = static_cast<float>(iconCol * kItemIconSize);
        float sy = static_cast<float>(atlasRow * kItemIconSize);
        source = Rectangle{sx, sy, static_cast<float>(kItemIconSize), static_cast<float>(kItemIconSize)};

        if ((sx + kItemIconSize) > static_cast<float>(iconAtlas.width) ||
            (sy + kItemIconSize) > static_cast<float>(iconAtlas.height)) {
            return false;
        }

        return true;
    }

    bool TryGetItemIconSource(const Item& item, Rectangle& source) const {
        if (!iconAtlasLoaded) return false;

        auto tierCol = [&](ItemTier tier) {
            switch (tier) {
            case ItemTier::Common: return 0;    // bronze
            case ItemTier::Uncommon: return 1;  // iron
            case ItemTier::Rare: return 2;      // steel
            case ItemTier::Mythic: return 3;    // mithril
            case ItemTier::Legendary: return 4; // vorpal
            default: return 0;
            }
        };

        auto armorStyleCol = [&]() {
            if (item.primaryStatType == 0) return 0; // metal
            if (item.primaryStatType == 1) return 1; // leather
            if (item.primaryStatType == 2) return 2; // cloth

            if (item.strBonus >= item.dexBonus && item.strBonus >= item.intBonus) return 0; // metal
            if (item.dexBonus >= item.strBonus && item.dexBonus >= item.intBonus) return 1; // leather
            return 2; // cloth
        };

        int atlasCol = 0;
        int atlasRow = 0;

        if (item.category == ItemCategory::Consumable) {
            atlasRow = 10;
            switch (item.consumableType) {
            case ConsumableType::HealthPotion: atlasCol = 0; break;
            case ConsumableType::Antidote: atlasCol = 1; break;
            case ConsumableType::ArmorPotion: atlasCol = 2; break;
            case ConsumableType::Bandage: atlasCol = 3; break;
            default: return false;
            }
        } else {
            switch (item.slot) {
            case EquipSlot::Weapon:
                atlasCol = tierCol(item.tier);
                if (item.weaponType == WeaponType::Sword) atlasRow = 0;
                else if (item.weaponType == WeaponType::Bow) atlasRow = 1;
                else atlasRow = 2;
                break;
            case EquipSlot::Head:
                atlasRow = 3;
                atlasCol = armorStyleCol();
                break;
            case EquipSlot::Top:
                atlasRow = 4;
                atlasCol = armorStyleCol();
                break;
            case EquipSlot::Hands:
                atlasRow = 5;
                atlasCol = armorStyleCol();
                break;
            case EquipSlot::Feet:
                atlasRow = 6;
                atlasCol = armorStyleCol();
                break;
            case EquipSlot::Shield:
                atlasRow = 7;
                atlasCol = tierCol(item.tier);
                break;
            case EquipSlot::Ring1:
            case EquipSlot::Ring2:
                atlasRow = 8;
                atlasCol = tierCol(item.tier);
                break;
            case EquipSlot::Necklace:
                atlasRow = 9;
                atlasCol = tierCol(item.tier);
                break;
            default:
                return false;
            }
        }

        float sx = static_cast<float>(atlasCol * kItemIconSize);
        float sy = static_cast<float>(atlasRow * kItemIconSize);
        source = Rectangle{sx, sy, static_cast<float>(kItemIconSize), static_cast<float>(kItemIconSize)};

        if ((sx + kItemIconSize) > static_cast<float>(iconAtlas.width) ||
            (sy + kItemIconSize) > static_cast<float>(iconAtlas.height)) {
            return false;
        }

        return true;
    }

    WeaponType CurrentWeaponType() const {
        const std::optional<Item>& weaponItem = player.equipped[SlotIndex(EquipSlot::Weapon)];
        if (weaponItem.has_value()) {
            return weaponItem->weaponType;
        }
        return WeaponType::Sword;
    }

    bool IsTileOpaque(int x, int y) const {
        if (!InBounds(x, y)) return true;
        TileType tile = tiles[Index(x, y)];
        return tile == TileType::Wall || tile == TileType::Outside;
    }

    bool IsSolidDecorationAt(int x, int y) const {
        for (const Decoration& decoration : solidDecorations) {
            if (decoration.x == x && decoration.y == y) return true;
        }
        return false;
    }

    bool IsAnyDecorationAt(int x, int y) const {
        for (const Decoration& decoration : walkableDecorations) {
            if (decoration.x == x && decoration.y == y) return true;
        }
        for (const Decoration& decoration : solidDecorations) {
            if (decoration.x == x && decoration.y == y) return true;
        }
        return false;
    }

    bool IsBlockingInteractableAt(int x, int y) const {
        for (const Interactable& interactable : interactables) {
            bool blocksWhenClosed = interactable.type == InteractableType::Chest || interactable.type == InteractableType::Pot;
            if (blocksWhenClosed && !interactable.opened && interactable.x == x && interactable.y == y) return true;
        }
        return false;
    }

    bool IsAnyInteractableAt(int x, int y) const {
        for (const Interactable& interactable : interactables) {
            if (interactable.x == x && interactable.y == y) return true;
        }
        return false;
    }

    void AddGoldPickupAt(int x, int y) {
        GoldStack rolled = RollGoldStackAt(x, y);
        for (GoldStack& existing : goldStacks) {
            if (existing.x == x && existing.y == y) {
                existing.amount += rolled.amount;
                if (existing.amount >= 51) existing.type = GoldStackType::Large;
                else if (existing.amount >= 11) existing.type = GoldStackType::Medium;
                else existing.type = GoldStackType::Small;
                return;
            }
        }
        goldStacks.push_back(rolled);
    }

    bool TryInteractWithNearbyActions() {
        const int dirs[5][2] = {
            {0, 0},
            {1, 0},
            {-1, 0},
            {0, 1},
            {0, -1}
        };

        for (const auto& d : dirs) {
            int tx = player.x + d[0];
            int ty = player.y + d[1];

            for (Interactable& interactable : interactables) {
                if (interactable.opened) continue;
                if (interactable.x != tx || interactable.y != ty) continue;
                if (interactable.type == InteractableType::Button || interactable.type == InteractableType::Spike) continue;

                interactable.opened = true;
                if (interactable.type == InteractableType::Chest) {
                    Item drop = CreateRandomEquipmentDrop(RollItemTier());
                    AddItemToInventory(drop);
                    AddGoldPickupAt(interactable.x, interactable.y);
                    PlayRandomSfx({
                        {&sfxChestOpen, &sfxChestOpenLoaded},
                        {&sfxChestOpen2, &sfxChestOpen2Loaded},
                        {&sfxChestOpen3, &sfxChestOpen3Loaded},
                        {&sfxChestOpen4, &sfxChestOpen4Loaded}
                    }, true, 0.95f, 1.06f);
                    AddLog("Opened chest: " + drop.name + " + gold.");
                } else {
                    Item drop = CreateRandomConsumableDrop();
                    AddItemToInventory(drop);
                    AddGoldPickupAt(interactable.x, interactable.y);
                    PlayRandomSfx({
                        {&sfxChestOpen, &sfxChestOpenLoaded},
                        {&sfxChestOpen2, &sfxChestOpen2Loaded},
                        {&sfxChestOpen3, &sfxChestOpen3Loaded},
                        {&sfxChestOpen4, &sfxChestOpen4Loaded}
                    }, true, 0.95f, 1.06f);
                    AddLog("Smashed pot: " + drop.name + " + gold.");
                }

                return true;
            }
        }

        return false;
    }

    bool TryPressButtonAt(int x, int y) {
        for (Interactable& interactable : interactables) {
            if (interactable.type != InteractableType::Button) continue;
            if (interactable.x != x || interactable.y != y) continue;
            if (interactable.opened) return false;

            interactable.opened = true;
            if (merchantPresent && !merchantRoomOpened && merchantDoorX >= 0 && merchantDoorY >= 0) {
                tiles[Index(merchantDoorX, merchantDoorY)] = TileType::Floor;
                merchantRoomOpened = true;
                PlaySfx(sfxStairs, sfxStairsLoaded);
                AddLog("Button pressed. The Merchant room door unlocks.");
            } else {
                PlaySfx(sfxInteract, sfxInteractLoaded);
                AddLog("Button pressed.");
            }
            return true;
        }
        return false;
    }

    bool TryTriggerSpikeAtPlayer(int x, int y) {
        for (Interactable& interactable : interactables) {
            if (interactable.type != InteractableType::Spike) continue;
            if (interactable.opened) continue;
            if (interactable.x != x || interactable.y != y) continue;

            PassiveRuntimeEffects passiveEffects = ComputePassiveEffects();
            if (passiveEffects.conduitPct > 0.0f) {
                interactable.opened = true;
                int attackBoost = std::max(1, static_cast<int>(std::round(static_cast<float>(player.atk) * passiveEffects.conduitPct * 0.35f)));
                player.temporaryAttackBonus += attackBoost;
                player.temporaryAttackTurns = std::max(player.temporaryAttackTurns, 3);
                RecomputePlayerStats();
                AddLog("Conduit converts the hazard into power.");
                return true;
            }

            interactable.opened = true;
            int rawDamage = std::max(1, static_cast<int>(std::ceil(player.maxHp * 0.20f)));
            int damageTaken = std::max(1, rawDamage - player.armor);
            player.hp -= damageTaken;
            bool wasBleeding = player.bleeding;
            player.bleeding = true;

            if (player.armor > 0 && rawDamage > damageTaken) {
                int blocked = rawDamage - damageTaken;
                AddLog("Spikes spring up! You take " + std::to_string(damageTaken) + " (" + std::to_string(blocked) + " blocked).");
            } else {
                AddLog("Spikes spring up! You take " + std::to_string(damageTaken) + ".");
            }
            if (!wasBleeding) {
                TriggerBleedAfflictFlash();
                AddLog("You are Bleeding!");
            }
            PlayRandomSfx({
                {&sfxHurt, &sfxHurtLoaded},
                {&sfxHurt2, &sfxHurt2Loaded},
                {&sfxHurt3, &sfxHurt3Loaded}
            }, true, 0.94f, 1.05f);
            if (player.hp < 0) player.hp = 0;
            return true;
        }
        return false;
    }

    bool TryTriggerSpikeAtEnemy(Enemy& enemy) {
        if (!enemy.alive || enemy.isFlying) return false;

        for (Interactable& interactable : interactables) {
            if (interactable.type != InteractableType::Spike) continue;
            if (interactable.opened) continue;
            if (interactable.x != enemy.x || interactable.y != enemy.y) continue;

            interactable.opened = true;
            int trapDamage = std::max(1, static_cast<int>(std::ceil(enemy.maxHp * 0.20f)));
            enemy.hp -= trapDamage;
            if (enemy.hp <= 0) {
                enemy.hp = 0;
                enemy.alive = false;
                AddLog(std::string(EnemyName(enemy.archetype)) + " is impaled by spikes.");
                TryOpenMerchantRoom();
            }
            return true;
        }
        return false;
    }

    void DamageEnemyByPlayer(int enemyIndex, int damage) {
        if (enemyIndex < 0 || enemyIndex >= static_cast<int>(enemies.size())) return;
        Enemy& enemy = enemies[enemyIndex];
        if (!enemy.alive) return;

        PassiveRuntimeEffects passiveEffects = ComputePassiveEffects();

        enemy.hp -= damage;
        WeaponType weaponType = CurrentWeaponType();
        if (weaponType == WeaponType::Sword) {
            PlayRandomSfx({
                {&sfxAttack, &sfxAttackLoaded},
                {&sfxAttack2, &sfxAttack2Loaded},
                {&sfxAttack3, &sfxAttack3Loaded}
            }, true, 0.94f, 1.07f);
        } else if (weaponType == WeaponType::Bow) {
            PlaySfx(sfxAttackRanged, sfxAttackRangedLoaded, true, 0.95f, 1.06f);
        } else {
            PlaySfx(sfxAttackMagic, sfxAttackMagicLoaded, true, 0.96f, 1.04f);
        }
        PlayRandomSfx({
            {&sfxEnemyDamaged, &sfxEnemyDamagedLoaded},
            {&sfxEnemyDamaged2, &sfxEnemyDamaged2Loaded},
            {&sfxEnemyDamaged3, &sfxEnemyDamaged3Loaded}
        }, true, 0.95f, 1.05f);
        AddLog("You hit for " + std::to_string(damage) + " damage.");
        if (enemy.hp < 0) enemy.hp = 0;

        if (passiveEffects.lifeLeechPct > 0.0f && damage > 0) {
            int leech = std::max(1, static_cast<int>(std::round(static_cast<float>(damage) * passiveEffects.lifeLeechPct)));
            int oldHp = player.hp;
            player.hp = std::min(player.maxHp, player.hp + leech);
            if (player.hp > oldHp) {
                AddLog("Vampirism restores " + std::to_string(player.hp - oldHp) + " HP.");
            }
        }

        if (enemy.hp * 2 <= enemy.maxHp) {
            enemy.panicFlee = true;
            AddLog("Enemy panics and tries to flee!");
        }

        if (enemy.hp <= 0) {
            enemy.alive = false;
            PlaySfx(sfxDeath, sfxDeathLoaded);
            score += 10;
            GainXp(5 + dungeonLevel);
            AddLog("Enemy defeated.");
            MaybeDropItemFromEnemy();

            if (passiveEffects.discoveryChance > 0.0f && RollChance(passiveEffects.discoveryChance)) {
                AddGoldPickupAt(enemy.x, enemy.y);
                MaybeDropItemFromEnemy();
                AddLog("Discovery grants bonus loot.");
            }

            if (passiveEffects.contagionPct > 0.0f) {
                int splash = std::max(1, static_cast<int>(std::round(static_cast<float>(player.atk) * passiveEffects.contagionPct * 0.25f)));
                int spreadHits = 0;
                for (int i = 0; i < static_cast<int>(enemies.size()); ++i) {
                    if (i == enemyIndex) continue;
                    Enemy& other = enemies[i];
                    if (!other.alive) continue;
                    int dx = std::abs(other.x - enemy.x);
                    int dy = std::abs(other.y - enemy.y);
                    if (dx + dy != 1) continue;
                    other.hp -= splash;
                    if (other.hp <= 0) {
                        other.hp = 0;
                        other.alive = false;
                        score += 10;
                        GainXp(2 + dungeonLevel / 2);
                        MaybeDropItemFromEnemy();
                    }
                    spreadHits += 1;
                }
                if (spreadHits > 0) {
                    AddLog("Contagion spreads to nearby foes.");
                }
            }

            TryOpenMerchantRoom();
        }
    }

    bool AttackSwordDirection(int targetTileX, int targetTileY) {
        int relX = targetTileX - player.x;
        int relY = targetTileY - player.y;
        if (relX == 0 && relY == 0) return false;

        int dirX = 0;
        int dirY = 0;
        if (std::abs(relX) >= std::abs(relY)) {
            dirX = relX > 0 ? 1 : -1;
        } else {
            dirY = relY > 0 ? 1 : -1;
        }

        std::vector<std::pair<int, int>> tilesToHit;
        if (dirX == 1) tilesToHit = {{player.x + 1, player.y - 1}, {player.x + 1, player.y}, {player.x + 1, player.y + 1}};
        if (dirX == -1) tilesToHit = {{player.x - 1, player.y - 1}, {player.x - 1, player.y}, {player.x - 1, player.y + 1}};
        if (dirY == 1) tilesToHit = {{player.x - 1, player.y + 1}, {player.x, player.y + 1}, {player.x + 1, player.y + 1}};
        if (dirY == -1) tilesToHit = {{player.x - 1, player.y - 1}, {player.x, player.y - 1}, {player.x + 1, player.y - 1}};

        for (const auto& tile : tilesToHit) {
            int enemyIndex = -1;
            if (IsOccupiedByEnemy(tile.first, tile.second, &enemyIndex)) {
                DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk));
            }
        }
        return true;
    }

    bool AttackBowRay(int targetTileX, int targetTileY) {
        int relX = targetTileX - player.x;
        int relY = targetTileY - player.y;
        if (relX == 0 && relY == 0) return false;

        int stepX = (relX > 0) ? 1 : (relX < 0 ? -1 : 0);
        int stepY = (relY > 0) ? 1 : (relY < 0 ? -1 : 0);

        int x = player.x + stepX;
        int y = player.y + stepY;
        while (InBounds(x, y) && !IsTileOpaque(x, y)) {
            int enemyIndex = -1;
            if (IsOccupiedByEnemy(x, y, &enemyIndex)) {
                DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk));
                break;
            }
            x += stepX;
            y += stepY;
        }
        return true;
    }

    bool AttackStaffPlus(int targetTileX, int targetTileY) {
        if (player.staffCharge < 5) {
            AddLog("Staff is recharging. Move " + std::to_string(5 - player.staffCharge) + " more tile(s).");
            return false;
        }

        if (!InBounds(targetTileX, targetTileY) || IsTileOpaque(targetTileX, targetTileY)) {
            return false;
        }
        if (!HasLineOfSight(player.x, player.y, targetTileX, targetTileY)) {
            AddLog("No line of sight for staff cast.");
            return false;
        }

        std::array<std::pair<int, int>, 5> tilesToHit = {{
            {targetTileX, targetTileY},
            {targetTileX + 1, targetTileY},
            {targetTileX - 1, targetTileY},
            {targetTileX, targetTileY + 1},
            {targetTileX, targetTileY - 1}
        }};

        for (const auto& tile : tilesToHit) {
            if (!InBounds(tile.first, tile.second)) continue;
            int enemyIndex = -1;
            if (IsOccupiedByEnemy(tile.first, tile.second, &enemyIndex)) {
                DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk));
            }
        }

        player.staffCharge = 0;
        return true;
    }

    bool TryMouseWeaponAttack() {
        int targetTileX = 0;
        int targetTileY = 0;
        if (!TryGetMouseWorldTile(targetTileX, targetTileY)) {
            return false;
        }

        WeaponType weaponType = CurrentWeaponType();
        if (weaponType == WeaponType::Sword) {
            return AttackSwordDirection(targetTileX, targetTileY);
        }
        if (weaponType == WeaponType::Bow) {
            return AttackBowRay(targetTileX, targetTileY);
        }
        return AttackStaffPlus(targetTileX, targetTileY);
    }

    std::vector<std::pair<int, int>> GetAttackPreviewTiles() const {
        std::vector<std::pair<int, int>> preview;

        int targetTileX = 0;
        int targetTileY = 0;
        if (!TryGetMouseWorldTile(targetTileX, targetTileY)) {
            return preview;
        }

        WeaponType weaponType = CurrentWeaponType();
        if (weaponType == WeaponType::Sword) {
            int relX = targetTileX - player.x;
            int relY = targetTileY - player.y;
            if (relX == 0 && relY == 0) return preview;
            int dirX = 0;
            int dirY = 0;
            if (std::abs(relX) >= std::abs(relY)) {
                dirX = relX > 0 ? 1 : -1;
            } else {
                dirY = relY > 0 ? 1 : -1;
            }
            if (dirX == 1) preview = {{player.x + 1, player.y - 1}, {player.x + 1, player.y}, {player.x + 1, player.y + 1}};
            if (dirX == -1) preview = {{player.x - 1, player.y - 1}, {player.x - 1, player.y}, {player.x - 1, player.y + 1}};
            if (dirY == 1) preview = {{player.x - 1, player.y + 1}, {player.x, player.y + 1}, {player.x + 1, player.y + 1}};
            if (dirY == -1) preview = {{player.x - 1, player.y - 1}, {player.x, player.y - 1}, {player.x + 1, player.y - 1}};
            return preview;
        }

        if (weaponType == WeaponType::Bow) {
            int relX = targetTileX - player.x;
            int relY = targetTileY - player.y;
            if (relX == 0 && relY == 0) return preview;
            int stepX = (relX > 0) ? 1 : (relX < 0 ? -1 : 0);
            int stepY = (relY > 0) ? 1 : (relY < 0 ? -1 : 0);
            int x = player.x + stepX;
            int y = player.y + stepY;
            while (InBounds(x, y) && !IsTileOpaque(x, y)) {
                preview.push_back({x, y});
                x += stepX;
                y += stepY;
            }
            return preview;
        }

        if (InBounds(targetTileX, targetTileY) && HasLineOfSight(player.x, player.y, targetTileX, targetTileY)) {
            preview.push_back({targetTileX, targetTileY});
            preview.push_back({targetTileX + 1, targetTileY});
            preview.push_back({targetTileX - 1, targetTileY});
            preview.push_back({targetTileX, targetTileY + 1});
            preview.push_back({targetTileX, targetTileY - 1});
        }
        return preview;
    }

    const char* ItemTierLabel(ItemTier tier) const {
        switch (tier) {
        case ItemTier::Common: return "Common";
        case ItemTier::Uncommon: return "Uncommon";
        case ItemTier::Rare: return "Rare";
        case ItemTier::Mythic: return "Mythic";
        case ItemTier::Legendary: return "Legendary";
        default: return "Common";
        }
    }

    Color ItemTierColor(ItemTier tier) const {
        switch (tier) {
        case ItemTier::Common: return GRAY;
        case ItemTier::Uncommon: return GREEN;
        case ItemTier::Rare: return BLUE;
        case ItemTier::Mythic: return RED;
        case ItemTier::Legendary: return GOLD;
        default: return GRAY;
        }
    }

    void HandleSkillTreeInput() {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_K)) {
            PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            skillTreeOpen = false;
            return;
        }

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            skillSelection -= 1;
            if (skillSelection < 0) skillSelection = kClassSkillCount - 1;
        } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            skillSelection += 1;
            if (skillSelection >= kClassSkillCount) skillSelection = 0;
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE)) {
            if (TrySpendSkillPoint(skillSelection)) {
                PlaySfx(sfxInteract, sfxInteractLoaded);
            } else {
                PlaySfx(sfxDecline, sfxDeclineLoaded);
                if (player.skillPoints <= 0) {
                    AddLog("No skill points available.");
                } else {
                    AddLog("Skill is already maxed.");
                }
            }
        }
    }

    void HandlePassiveChoiceInput() {
        if (!passiveChoiceOpen) return;

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            passiveChoiceSelection -= 1;
            if (passiveChoiceSelection < 0) passiveChoiceSelection = 2;
            PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
        } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            passiveChoiceSelection += 1;
            if (passiveChoiceSelection > 2) passiveChoiceSelection = 0;
            PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
        }

        if (IsKeyPressed(KEY_ONE)) {
            ApplyPassiveChoice(0);
            PlaySfx(sfxInteract, sfxInteractLoaded);
            return;
        }
        if (IsKeyPressed(KEY_TWO)) {
            ApplyPassiveChoice(1);
            PlaySfx(sfxInteract, sfxInteractLoaded);
            return;
        }
        if (IsKeyPressed(KEY_THREE)) {
            ApplyPassiveChoice(2);
            PlaySfx(sfxInteract, sfxInteractLoaded);
            return;
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE)) {
            ApplyPassiveChoice(passiveChoiceSelection);
            PlaySfx(sfxInteract, sfxInteractLoaded);
            return;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            const int panelW = 960;
            const int panelH = 420;
            const int panelX = (kScreenWidth - panelW) / 2;
            const int panelY = (kScreenHeight - panelH) / 2;
            const int cardW = 280;
            const int cardH = 220;
            const int gap = 26;
            const int totalCardsW = cardW * 3 + gap * 2;
            const int startX = panelX + (panelW - totalCardsW) / 2;
            const int cardY = panelY + 124;

            Vector2 mousePos = GetMousePosition();
            for (int i = 0; i < 3; ++i) {
                int x = startX + i * (cardW + gap);
                Rectangle cardRect{static_cast<float>(x), static_cast<float>(cardY), static_cast<float>(cardW), static_cast<float>(cardH)};
                if (!CheckCollisionPointRec(mousePos, cardRect)) continue;
                ApplyPassiveChoice(i);
                PlaySfx(sfxInteract, sfxInteractLoaded);
                return;
            }
        }
    }

    void DrawActiveSkillHotbar() const {
        if (classSelectionPending || inventoryOpen || merchantOpen || pauseMenuOpen || skillTreeOpen || gameOver || devConsoleOpen) {
            return;
        }

        const int mapW = kMapWidth * kTileSize;
        const int mapH = kMapHeight * kTileSize;
        const int slotCount = 10;
        const int slotSize = 52;
        const int slotGap = 6;
        const int totalW = slotCount * slotSize + (slotCount - 1) * slotGap;
        const int startX = (mapW - totalW) / 2;
        const int y = mapH - slotSize - 8;

        const char* keyLabels[10] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};

        for (int i = 0; i < slotCount; ++i) {
            int x = startX + i * (slotSize + slotGap);
            Rectangle rect{static_cast<float>(x), static_cast<float>(y), static_cast<float>(slotSize), static_cast<float>(slotSize)};

            ActiveSkillId skillId = player.activeSkillLoadout[static_cast<size_t>(i)];
            int requiredBranch = ActiveSkillPassiveIndex(skillId);
            bool unlocked = (skillId != ActiveSkillId::None) && (requiredBranch < 0 || player.classSkillRanks[requiredBranch] >= kClassSkillMaxRank);
            int cooldown = player.activeSkillCooldowns[static_cast<size_t>(i)];

            Color fill = Color{20, 24, 34, 190};
            if (unlocked && cooldown <= 0) fill = Color{42, 62, 98, 210};
            if (unlocked && cooldown > 0) fill = Color{52, 46, 68, 210};

            DrawRectangleRec(rect, fill);
            DrawRectangleLinesEx(rect, 2.0f, Color{95, 110, 145, 255});

            DrawText(keyLabels[i], x + 6, y + 4, 16, Color{210, 220, 245, 255});

            const char* skillLabel = "-";
            if (skillId != ActiveSkillId::None) {
                skillLabel = unlocked ? ActiveSkillName(skillId) : "Locked";
            }

            int fontSize = 12;
            while (fontSize > 9 && MeasureText(skillLabel, fontSize) > (slotSize - 8)) {
                fontSize -= 1;
            }

            Color textColor = unlocked ? Color{220, 235, 255, 255} : Color{160, 120, 120, 255};
            int textW = MeasureText(skillLabel, fontSize);
            DrawText(skillLabel, x + (slotSize - textW) / 2, y + slotSize - fontSize - 5, fontSize, textColor);

            if (unlocked && cooldown > 0) {
                DrawText(TextFormat("%i", cooldown), x + slotSize - 18, y + 4, 16, Color{255, 220, 130, 255});
            }
        }
    }

    void ClampInventorySelection() {
        if (player.inventory.empty()) {
            inventorySelection = 0;
            return;
        }

        if (inventorySelection < 0) inventorySelection = 0;
        int maxIndex = static_cast<int>(player.inventory.size()) - 1;
        if (inventorySelection > maxIndex) inventorySelection = maxIndex;
    }

    std::string InventoryItemLine(const Item& item) const {
        std::string line = item.name;

        if (item.category == ItemCategory::Equipment) {
            line += " [";
            line += EquipSlotLabel(item.slot);
            line += "]";
            if (item.strBonus != 0) line += " STR+" + std::to_string(item.strBonus);
            if (item.dexBonus != 0) line += " DEX+" + std::to_string(item.dexBonus);
            if (item.intBonus != 0) line += " INT+" + std::to_string(item.intBonus);
            if (item.hpBonus != 0) line += " HP+" + std::to_string(item.hpBonus);
            if (item.armorBonus != 0) line += " ARM+" + std::to_string(item.armorBonus);
        } else {
            line += " x" + std::to_string(item.quantity);
        }

        return line;
    }

    void HandleInventoryInput() {
        if (IsKeyPressed(KEY_ESCAPE)) {
            PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            inventoryOpen = false;
            return;
        }

        const int panelX = 90;
        const int panelY = 56;
        const int panelW = kScreenWidth - 180;
        const int panelH = kScreenHeight - 112;

        const int equipX = panelX + 20;
        const int equipY = panelY + 92;
        const int equipW = panelW - 40;
        const int equipH = 182;

        const int listX = panelX + 20;
        const int listY = equipY + equipH + 14;
        const int listH = panelY + panelH - listY - 16;
        const int gridTop = listY + 42;

        const int visibleRows = std::max(1, (listH - 52) / (kInventoryCellSize + kInventoryCellGap));
        const int totalRows = (static_cast<int>(player.inventory.size()) + kInventoryGridCols - 1) / kInventoryGridCols;
        const int maxScrollRows = std::max(0, totalRows - visibleRows);

        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            inventoryScrollOffset += (wheel < 0.0f) ? 1 : -1;
            inventoryScrollOffset = std::clamp(inventoryScrollOffset, 0, maxScrollRows);
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();

            const EquipSlot displaySlots[9] = {
                EquipSlot::Head, EquipSlot::Top, EquipSlot::Feet, EquipSlot::Hands,
                EquipSlot::Ring1, EquipSlot::Ring2, EquipSlot::Necklace, EquipSlot::Weapon, EquipSlot::Shield
            };

            int slotLine = 0;
            for (EquipSlot slot : displaySlots) {
                int column = slotLine % 2;
                int row = slotLine / 2;
                int lineX = equipX + 14 + column * (equipW / 2);
                int lineY = equipY + 40 + row * 26;
                Rectangle slotRect{static_cast<float>(lineX - 6), static_cast<float>(lineY - 2), static_cast<float>(equipW / 2 - 24), 24.0f};

                if (CheckCollisionPointRec(mousePos, slotRect)) {
                    if (UnequipSlot(slot)) {
                        ClampInventorySelection();
                    }
                    return;
                }
                slotLine += 1;
            }

            if (!player.inventory.empty()) {
                for (int row = 0; row < visibleRows; ++row) {
                    for (int col = 0; col < kInventoryGridCols; ++col) {
                        int index = (inventoryScrollOffset + row) * kInventoryGridCols + col;
                        if (index >= static_cast<int>(player.inventory.size())) continue;

                        int cellX = listX + 10 + col * (kInventoryCellSize + kInventoryCellGap);
                        int cellY = gridTop + row * (kInventoryCellSize + kInventoryCellGap);
                        Rectangle cellRect{static_cast<float>(cellX), static_cast<float>(cellY), static_cast<float>(kInventoryCellSize), static_cast<float>(kInventoryCellSize)};

                        if (CheckCollisionPointRec(mousePos, cellRect)) {
                            inventorySelection = index;

                            ItemCategory category = player.inventory[inventorySelection].category;
                            bool success = false;
                            if (category == ItemCategory::Equipment) {
                                success = EquipInventoryItem(static_cast<size_t>(inventorySelection));
                            } else {
                                success = UseConsumableInventoryItem(static_cast<size_t>(inventorySelection));
                            }

                            if (!success) AddLog("Cannot use selected item.");
                            ClampInventorySelection();
                            return;
                        }
                    }
                }
            }
        }

        if (player.inventory.empty()) return;

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
            inventorySelection -= 1;
        } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
            inventorySelection += 1;
        } else if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            inventorySelection -= kInventoryGridCols;
        } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            inventorySelection += kInventoryGridCols;
        }

        inventorySelection = std::clamp(inventorySelection, 0, static_cast<int>(player.inventory.size()) - 1);

        int selectedRow = inventorySelection / kInventoryGridCols;
        if (selectedRow < inventoryScrollOffset) inventoryScrollOffset = selectedRow;
        if (selectedRow >= inventoryScrollOffset + visibleRows) inventoryScrollOffset = selectedRow - visibleRows + 1;
        inventoryScrollOffset = std::clamp(inventoryScrollOffset, 0, maxScrollRows);

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E)) {
            ItemCategory category = player.inventory[inventorySelection].category;
            bool success = false;
            if (category == ItemCategory::Equipment) {
                success = EquipInventoryItem(static_cast<size_t>(inventorySelection));
            } else {
                success = UseConsumableInventoryItem(static_cast<size_t>(inventorySelection));
            }

            if (!success) AddLog("Cannot use selected item.");
            ClampInventorySelection();
        }
    }

    void HandleMerchantInput() {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_I)) {
            PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            merchantOpen = false;
            return;
        }

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
            PlaySfx(sfxDecline, sfxDeclineLoaded);
            if (merchantTab == MerchantTab::Buy) merchantTab = MerchantTab::Sell;
            else if (merchantTab == MerchantTab::Reroll) merchantTab = MerchantTab::Buy;
            else merchantTab = MerchantTab::Reroll;
            merchantSelection = 0;
        } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
            PlaySfx(sfxInteract, sfxInteractLoaded);
            if (merchantTab == MerchantTab::Buy) merchantTab = MerchantTab::Reroll;
            else if (merchantTab == MerchantTab::Reroll) merchantTab = MerchantTab::Sell;
            else merchantTab = MerchantTab::Buy;
            merchantSelection = 0;
        }

        int count = 0;
        if (merchantTab == MerchantTab::Buy) {
            count = static_cast<int>(merchantStock.size());
        } else if (merchantTab == MerchantTab::Reroll) {
            count = static_cast<int>(MerchantInventoryIndices(true).size());
        } else {
            count = static_cast<int>(MerchantInventoryIndices(false).size());
        }

        if (count > 0) {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) merchantSelection -= 1;
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) merchantSelection += 1;
            merchantSelection = std::clamp(merchantSelection, 0, count - 1);
        } else {
            merchantSelection = 0;
        }

        if (!(IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E))) return;
        PlaySfx(sfxInteract, sfxInteractLoaded);

        if (merchantTab == MerchantTab::Buy) {
            if (merchantStock.empty()) {
                AddLog("Merchant has nothing to sell.");
                return;
            }

            int index = std::clamp(merchantSelection, 0, static_cast<int>(merchantStock.size()) - 1);
            Item item = merchantStock[index];
            int cost = ComputeBuyValue(item);
            if (player.gold < cost) {
                AddLog("Not enough gold.");
                return;
            }

            player.gold -= cost;
            TriggerCharityBuffIfApplicable();
            AddItemToInventory(item);
            merchantStock.erase(merchantStock.begin() + static_cast<std::vector<Item>::difference_type>(index));
            if (!merchantStock.empty()) {
                merchantSelection = std::clamp(merchantSelection, 0, static_cast<int>(merchantStock.size()) - 1);
            } else {
                merchantSelection = 0;
            }
            PlaySfx(sfxGold, sfxGoldLoaded);
            AddLog("Bought " + item.name + " for " + std::to_string(cost) + " gold.");
            return;
        }

        if (merchantTab == MerchantTab::Reroll) {
            std::vector<int> equipmentIndices = MerchantInventoryIndices(true);
            if (equipmentIndices.empty()) {
                AddLog("No equipment in inventory to reroll.");
                return;
            }

            int selected = std::clamp(merchantSelection, 0, static_cast<int>(equipmentIndices.size()) - 1);
            int inventoryIndex = equipmentIndices[selected];
            Item& item = player.inventory[inventoryIndex];
            int cost = ComputeRerollCost(item);
            if (player.gold < cost) {
                AddLog("Not enough gold.");
                return;
            }

            player.gold -= cost;
            TriggerCharityBuffIfApplicable();
            ApplyRandomEquipmentRoll(item);
            PlaySfx(sfxGold, sfxGoldLoaded);
            AddLog("Rerolled " + item.name + " for " + std::to_string(cost) + " gold.");
            return;
        }

        std::vector<int> inventoryIndices = MerchantInventoryIndices(false);
        if (inventoryIndices.empty()) {
            AddLog("Inventory is empty.");
            return;
        }

        int selected = std::clamp(merchantSelection, 0, static_cast<int>(inventoryIndices.size()) - 1);
        int inventoryIndex = inventoryIndices[selected];
        const Item item = player.inventory[inventoryIndex];
        int value = ComputeSellValue(item);

        player.gold += value;
        player.inventory.erase(player.inventory.begin() + static_cast<std::vector<Item>::difference_type>(inventoryIndex));

        inventorySelection = 0;
        if (!player.inventory.empty()) {
            inventorySelection = std::clamp(inventorySelection, 0, static_cast<int>(player.inventory.size()) - 1);
        }

        if (!inventoryIndices.empty()) {
            merchantSelection = std::clamp(merchantSelection, 0, std::max(0, static_cast<int>(inventoryIndices.size()) - 2));
        }
        PlaySfx(sfxGold, sfxGoldLoaded);
        AddLog("Sold " + item.name + " for " + std::to_string(value) + " gold.");
    }

    size_t SlotIndex(EquipSlot slot) const {
        return static_cast<size_t>(slot);
    }

    void RecomputePlayerStats() {
        player.strength = player.baseStrength;
        player.dexterity = player.baseDexterity;
        player.intelligence = player.baseIntelligence;
        player.maxHp = player.baseMaxHp;
        player.armor = 0;
        int weaponDamageModifier = 0;
        int defenseModifier = 0;

        for (const auto& eq : player.equipped) {
            if (!eq.has_value()) continue;
            const Item& item = eq.value();
            player.strength += item.strBonus;
            player.dexterity += item.dexBonus;
            player.intelligence += item.intBonus;
            player.maxHp += item.hpBonus;
            player.armor += item.armorBonus;

            int powerModifier = static_cast<int>(std::round(item.itemPower));
            if (item.slot == EquipSlot::Weapon) {
                weaponDamageModifier += powerModifier;
            } else {
                defenseModifier += powerModifier;
            }
        }

        int skillMaxHpBonus = 0;
        int skillArmorBonus = 0;
        int skillStrengthBonus = 0;
        int skillDexterityBonus = 0;
        int skillIntelligenceBonus = 0;
        int skillAttackBonus = 0;
        GetSkillBonuses(skillMaxHpBonus, skillArmorBonus, skillStrengthBonus, skillDexterityBonus, skillIntelligenceBonus, skillAttackBonus);

        player.maxHp += skillMaxHpBonus;
        player.armor += skillArmorBonus;
        player.strength += skillStrengthBonus;
        player.dexterity += skillDexterityBonus;
        player.intelligence += skillIntelligenceBonus;
        PassiveRuntimeEffects passiveEffects = ComputePassiveEffects();

        player.maxHp = std::max(1, static_cast<int>(std::round(static_cast<float>(player.maxHp) * std::max(0.10f, 1.0f + passiveEffects.maxHpPct))));
        player.strength = std::max(1, static_cast<int>(std::round(static_cast<float>(player.strength) * std::max(0.10f, 1.0f + passiveEffects.strengthPct))));
        player.dexterity = std::max(1, static_cast<int>(std::round(static_cast<float>(player.dexterity) * std::max(0.10f, 1.0f + passiveEffects.dexterityPct))));
        player.intelligence = std::max(1, static_cast<int>(std::round(static_cast<float>(player.intelligence) * std::max(0.10f, 1.0f + passiveEffects.intelligencePct))));
        player.armor += player.temporaryArmorBonus;

        player.armor += defenseModifier;
        player.armor = std::max(0, static_cast<int>(std::round(static_cast<float>(player.armor) * std::max(0.10f, 1.0f + passiveEffects.armorPct))));

        if (player.hp > player.maxHp) {
            player.hp = player.maxHp;
        }

        int rawAttack = player.strength + weaponDamageModifier + skillAttackBonus + player.temporaryAttackBonus;
        player.atk = std::max(1, static_cast<int>(std::round(static_cast<float>(rawAttack) * std::max(0.10f, passiveEffects.damageMult))));
    }

    void AddItemToInventory(const Item& item) {
        if (item.stackable) {
            for (auto& existing : player.inventory) {
                if (existing.stackable && existing.category == item.category && existing.name == item.name) {
                    existing.quantity += item.quantity;
                    return;
                }
            }
        }

        player.inventory.push_back(item);
    }

    bool EquipInventoryItem(size_t invIndex) {
        if (invIndex >= player.inventory.size()) return false;

        Item item = player.inventory[invIndex];
        if (item.category != ItemCategory::Equipment) return false;

        if (player.level < item.itemLevel) {
            AddLog("Requires level " + std::to_string(item.itemLevel) + " to equip.");
            return false;
        }

        if (item.slot == EquipSlot::Ring1 || item.slot == EquipSlot::Ring2) {
            size_t ring1Index = SlotIndex(EquipSlot::Ring1);
            size_t ring2Index = SlotIndex(EquipSlot::Ring2);

            size_t targetSlot = ring1Index;
            if (!player.equipped[ring1Index].has_value()) {
                targetSlot = ring1Index;
            } else if (!player.equipped[ring2Index].has_value()) {
                targetSlot = ring2Index;
            }

            if (player.equipped[targetSlot].has_value()) {
                AddItemToInventory(player.equipped[targetSlot].value());
            }

            item.slot = (targetSlot == ring2Index) ? EquipSlot::Ring2 : EquipSlot::Ring1;
            player.equipped[targetSlot] = item;
        } else {
            size_t slot = SlotIndex(item.slot);
            if (player.equipped[slot].has_value()) {
                AddItemToInventory(player.equipped[slot].value());
            }
            player.equipped[slot] = item;
        }

        player.inventory.erase(player.inventory.begin() + static_cast<std::vector<Item>::difference_type>(invIndex));
        RecomputePlayerStats();
        AddLog("Equipped: " + item.name);
        return true;
    }

    bool UnequipSlot(EquipSlot slot) {
        size_t slotIndex = SlotIndex(slot);
        if (!player.equipped[slotIndex].has_value()) return false;

        Item item = player.equipped[slotIndex].value();
        if (item.slot == EquipSlot::Ring1 || item.slot == EquipSlot::Ring2) {
            item.slot = EquipSlot::Ring1;
        }
        AddItemToInventory(item);
        player.equipped[slotIndex].reset();
        RecomputePlayerStats();
        AddLog("Unequipped: " + item.name);
        return true;
    }

    bool UseConsumableInventoryItem(size_t invIndex) {
        if (invIndex >= player.inventory.size()) return false;

        Item& item = player.inventory[invIndex];
        if (item.category != ItemCategory::Consumable) return false;

        PlaySfx(sfxUseItem, sfxUseItemLoaded);

        switch (item.consumableType) {
        case ConsumableType::HealthPotion:
            player.hp = std::min(player.maxHp, player.hp + 20);
            AddLog("Used Health Potion (+20 HP).");
            break;
        case ConsumableType::ArmorPotion:
            player.armor += 5;
            AddLog("Used Armor Potion (+5 Armor).");
            break;
        case ConsumableType::Antidote:
            if (player.poisoned) {
                player.poisoned = false;
                AddLog("Used Antidote. Poison cured.");
            } else {
                AddLog("Used Antidote.");
            }
            break;
        case ConsumableType::Bandage:
            player.hp = std::min(player.maxHp, player.hp + 10);
            if (player.bleeding) {
                player.bleeding = false;
                AddLog("Used Bandage (+10 HP, Bleed cured).");
            } else {
                AddLog("Used Bandage (+10 HP).");
            }
            break;
        }

        item.quantity -= 1;
        if (item.quantity <= 0) {
            player.inventory.erase(player.inventory.begin() + static_cast<std::vector<Item>::difference_type>(invIndex));
        }

        TriggerCharityBuffIfApplicable();

        return true;
    }

    void ApplyStepStatusEffects() {
        if (player.poisoned && player.hp > 0) {
            int poisonDamage = std::max(1, player.hp / 10);
            player.hp -= poisonDamage;
            AddLog("Poisoned: " + std::to_string(poisonDamage) + " damage.");
        }

        if (player.bleeding && player.hp > 0) {
            int bleedDamage = std::max(1, player.maxHp / 20);
            player.hp -= bleedDamage;
            AddLog("Bleed: " + std::to_string(bleedDamage) + " damage.");
        }

        if (player.hp < 0) player.hp = 0;
    }

    void CheckGameOver() {
        if (!gameOver && player.hp <= 0) {
            PassiveRuntimeEffects effects = ComputePassiveEffects();
            if (effects.revivalChance > 0.0f && RollChance(effects.revivalChance)) {
                player.hp = player.maxHp;
                AddLog("Revival restores you to full health!");
                return;
            }
            player.hp = 0;
            gameOver = true;
            PlaySfx(sfxDeath, sfxDeathLoaded);
            AddLog("You died. Press R to start a new run.");
        }
    }

    int Index(int x, int y) const {
        return y * kMapWidth + x;
    }

    bool InBounds(int x, int y) const {
        return x >= 0 && y >= 0 && x < kMapWidth && y < kMapHeight;
    }

    bool IsWalkable(int x, int y) const {
        if (!InBounds(x, y)) return false;
        TileType t = tiles[Index(x, y)];
        if (!(t == TileType::Floor || t == TileType::Stairs)) return false;
        if (IsSolidDecorationAt(x, y)) return false;
        if (IsBlockingInteractableAt(x, y)) return false;
        return true;
    }

    bool HasPathToTile(int goalX, int goalY) const {
        if (!InBounds(player.x, player.y) || !InBounds(goalX, goalY)) return false;
        if (player.x == goalX && player.y == goalY) return true;

        std::vector<uint8_t> visited(kMapWidth * kMapHeight, 0);
        std::vector<std::pair<int, int>> queue;
        queue.reserve(kMapWidth * kMapHeight);

        visited[Index(player.x, player.y)] = 1;
        queue.push_back({player.x, player.y});

        size_t head = 0;
        const int dirs[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
        while (head < queue.size()) {
            int x = queue[head].first;
            int y = queue[head].second;
            ++head;

            for (const auto& d : dirs) {
                int nx = x + d[0];
                int ny = y + d[1];
                if (!InBounds(nx, ny)) continue;
                int idx = Index(nx, ny);
                if (visited[idx]) continue;
                if (!IsWalkable(nx, ny)) continue;

                if (nx == goalX && ny == goalY) return true;

                visited[idx] = 1;
                queue.push_back({nx, ny});
            }
        }

        return false;
    }

    void EnsureProgressionPathToStairs(int stairsX, int stairsY) {
        if (HasPathToTile(stairsX, stairsY)) return;

        solidDecorations.clear();
        for (Interactable& interactable : interactables) {
            if (interactable.type == InteractableType::Chest || interactable.type == InteractableType::Pot) {
                interactable.opened = true;
            }
        }

        if (HasPathToTile(stairsX, stairsY)) {
            AddLog("The dungeon shifts to keep a path open.");
            return;
        }

        CarveCorridor(player.x, player.y, stairsX, stairsY);
        BuildWallsFromOutside();

        if (!HasPathToTile(stairsX, stairsY)) {
            for (Interactable& interactable : interactables) {
                if (interactable.type == InteractableType::Chest || interactable.type == InteractableType::Pot) {
                    interactable.opened = true;
                }
            }
        }

        AddLog("The dungeon shifts to keep a path open.");
    }

    void BuildWallsFromOutside() {
        std::vector<TileType> next = tiles;
        const int dirs[8][2] = {
            { 1, 0 }, {-1, 0 }, { 0, 1 }, { 0,-1 },
            { 1, 1 }, { 1,-1 }, {-1, 1 }, {-1,-1 }
        };

        for (int y = 0; y < kMapHeight; ++y) {
            for (int x = 0; x < kMapWidth; ++x) {
                int idx = Index(x, y);
                if (tiles[idx] != TileType::Outside) continue;

                bool adjacentToInterior = false;
                for (const auto& d : dirs) {
                    int nx = x + d[0];
                    int ny = y + d[1];
                    if (!InBounds(nx, ny)) continue;

                    TileType neighbor = tiles[Index(nx, ny)];
                    if (neighbor == TileType::Floor || neighbor == TileType::Stairs) {
                        adjacentToInterior = true;
                        break;
                    }
                }

                if (adjacentToInterior) {
                    next[idx] = TileType::Wall;
                }
            }
        }

        tiles.swap(next);
    }

    bool IsEnemyAt(int x, int y, int ignoreEnemyIndex = -1) const {
        for (int i = 0; i < static_cast<int>(enemies.size()); ++i) {
            if (i == ignoreEnemyIndex) continue;
            if (!enemies[i].alive) continue;
            if (enemies[i].x == x && enemies[i].y == y) return true;
        }
        return false;
    }

    bool IsWalkableForEnemy(int x, int y, int selfEnemyIndex) const {
        if (!IsWalkable(x, y)) return false;
        if (IsEnemyAt(x, y, selfEnemyIndex)) return false; // prevent enemy-over-enemy pathing
        if (IsMerchantAt(x, y)) return false;
        if (IsMerchantChamberLockedTile(x, y)) return false;
        return true;
    }

    int Heuristic(int x1, int y1, int x2, int y2) const {
        return std::abs(x1 - x2) + std::abs(y1 - y2); // Manhattan
    }

    bool HasLineOfSight(int x0, int y0, int x1, int y1) const {
        // Bresenham line
        int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;

        while (true) {
            if (!(x0 == x1 && y0 == y1)) {
                if (!InBounds(x0, y0)) return false;
                if (tiles[Index(x0, y0)] == TileType::Wall) return false;
            }

            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
        return true;
    }

    std::vector<std::pair<int, int>> FindPathAStar(int sx, int sy, int gx, int gy, int selfEnemyIndex) const {
        std::vector<std::pair<int, int>> empty;
        if (!IsWalkableForEnemy(sx, sy, selfEnemyIndex) || !IsWalkable(gx, gy)) return empty;

        const int n = kMapWidth * kMapHeight;
        const int INF = std::numeric_limits<int>::max();

        std::vector<int> gScore(n, INF);
        std::vector<int> fScore(n, INF);
        std::vector<int> cameFrom(n, -1);
        std::vector<bool> open(n, false);
        std::vector<bool> closed(n, false);
        auto idx = [&](int x, int y) { return Index(x, y); };

        int start = idx(sx, sy);
        int goal = idx(gx, gy);

        gScore[start] = 0;
        fScore[start] = Heuristic(sx, sy, gx, gy);
        open[start] = true;

        const int dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };

        while (true) {
            int current = -1;
            int bestF = INF;

            for (int i = 0; i < n; ++i) {
                if (open[i] && !closed[i] && fScore[i] < bestF) {
                    bestF = fScore[i];
                    current = i;
                }
            }

            if (current == -1) return empty; // no path
            if (current == goal) break;

            open[current] = false;
            closed[current] = true;

            int cx = current % kMapWidth;
            int cy = current / kMapWidth;

            for (auto& d : dirs) {
                int nx = cx + d[0];
                int ny = cy + d[1];

                // allow goal tile even if "blocked" by occupancy rules
                if (!(nx == gx && ny == gy) && !IsWalkableForEnemy(nx, ny, selfEnemyIndex)) continue;

                int ni = idx(nx, ny);
                if (closed[ni]) continue;

                int tentativeG = gScore[current] + 1;
                if (tentativeG < gScore[ni]) {
                    cameFrom[ni] = current;
                    gScore[ni] = tentativeG;
                    fScore[ni] = tentativeG + Heuristic(nx, ny, gx, gy);
                    open[ni] = true;
                }
            }

        }

        std::vector<std::pair<int, int>> path;
        for (int cur = goal; cur != -1; cur = cameFrom[cur]) {
            int x = cur % kMapWidth;
            int y = cur / kMapWidth;
            path.push_back({x, y});
            if (cur == start) break;
        }
        std::reverse(path.begin(), path.end());
        return path;
    }

    void ApplyClassPreset(PlayerClass cls) {
        player.playerClass = cls;
        player.skillPoints = 0;
        player.classSkillRanks = {0, 0, 0};
        player.activeSkillCooldowns = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        player.temporaryArmorBonus = 0;
        player.temporaryArmorTurns = 0;
        player.temporaryAttackBonus = 0;
        player.temporaryAttackTurns = 0;
        player.activeSkillLoadout = {
            ActiveSkillId::None, ActiveSkillId::None, ActiveSkillId::None, ActiveSkillId::None, ActiveSkillId::None,
            ActiveSkillId::None, ActiveSkillId::None, ActiveSkillId::None, ActiveSkillId::None, ActiveSkillId::None
        };

        Item starterWeapon;
        starterWeapon.category = ItemCategory::Equipment;
        starterWeapon.tier = ItemTier::Common;
        starterWeapon.slot = EquipSlot::Weapon;
        starterWeapon.itemLevel = 1;
        starterWeapon.basePower = 3;
        starterWeapon.rarityMultiplier = 1.0f;
        starterWeapon.itemPower = 3.0f;

        switch (cls) {
        case PlayerClass::Warrior:
            player.baseMaxHp = 28;
            player.baseStrength = 10;
            player.baseDexterity = 5;
            player.baseIntelligence = 3;
            player.activeSkillLoadout[0] = ActiveSkillId::WarriorCleave;
            player.activeSkillLoadout[1] = ActiveSkillId::WarriorSecondWind;
            player.activeSkillLoadout[2] = ActiveSkillId::WarriorShieldWall;
            starterWeapon.name = "Broken Sword";
            starterWeapon.weaponType = WeaponType::Sword;
            break;
        case PlayerClass::Ranger:
            player.baseMaxHp = 22;
            player.baseStrength = 5;
            player.baseDexterity = 10;
            player.baseIntelligence = 3;
            player.activeSkillLoadout[0] = ActiveSkillId::RangerPiercingShot;
            player.activeSkillLoadout[1] = ActiveSkillId::RangerVolley;
            player.activeSkillLoadout[2] = ActiveSkillId::RangerSurvivalInstinct;
            starterWeapon.name = "Worn Bow";
            starterWeapon.weaponType = WeaponType::Bow;
            break;
        case PlayerClass::Wizard:
            player.baseMaxHp = 18;
            player.baseStrength = 3;
            player.baseDexterity = 5;
            player.baseIntelligence = 10;
            player.activeSkillLoadout[0] = ActiveSkillId::WizardArcaneNova;
            player.activeSkillLoadout[1] = ActiveSkillId::WizardBlink;
            player.activeSkillLoadout[2] = ActiveSkillId::WizardBarrier;
            starterWeapon.name = "Uncharged Staff";
            starterWeapon.weaponType = WeaponType::Staff;
            break;
        }

        player.hp = player.baseMaxHp;
        player.staffCharge = (cls == PlayerClass::Wizard) ? 0 : 5;
        player.inventory.clear();
        inventoryScrollOffset = 0;
        for (auto& s : player.equipped) s.reset();
        player.equipped[SlotIndex(EquipSlot::Weapon)] = starterWeapon;

        Item healthPotion;
        healthPotion.name = "Health Potion";
        healthPotion.category = ItemCategory::Consumable;
        healthPotion.tier = ItemTier::Common;
        healthPotion.consumableType = ConsumableType::HealthPotion;
        healthPotion.stackable = true;
        healthPotion.quantity = 2;
        AddItemToInventory(healthPotion);

        Item bandage;
        bandage.name = "Bandage";
        bandage.category = ItemCategory::Consumable;
        bandage.tier = ItemTier::Common;
        bandage.consumableType = ConsumableType::Bandage;
        bandage.stackable = true;
        bandage.quantity = 2;
        AddItemToInventory(bandage);

        AddLog("Starting weapon: " + starterWeapon.name);

        RecomputePlayerStats();
        skillTreeOpen = false;
        skillSelection = 0;
        classSelectionPending = false;
        GenerateLevel(true);
    }

    void AddLog(std::string message) {
        combatLog.push_back(std::move(message));
        constexpr int kMaxLogLines = 10;
        if (static_cast<int>(combatLog.size()) > kMaxLogLines) {
            combatLog.erase(combatLog.begin());
        }
    }

    void AddDevConsoleLine(const std::string& line) {
        devConsoleHistory.push_back(line);
        constexpr int kMaxConsoleLines = 8;
        if (static_cast<int>(devConsoleHistory.size()) > kMaxConsoleLines) {
            devConsoleHistory.erase(devConsoleHistory.begin());
        }
    }

    std::string ToLower(std::string value) const {
        for (char& c : value) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return value;
    }

    std::vector<std::string> TokenizeCommand(const std::string& input) const {
        std::vector<std::string> tokens;
        std::string current;
        for (char c : input) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current.push_back(c);
            }
        }
        if (!current.empty()) {
            tokens.push_back(current);
        }
        return tokens;
    }

    bool ParseItemTierToken(const std::string& token, ItemTier& outTier) const {
        std::string value = ToLower(token);
        if (value == "common") { outTier = ItemTier::Common; return true; }
        if (value == "uncommon") { outTier = ItemTier::Uncommon; return true; }
        if (value == "rare") { outTier = ItemTier::Rare; return true; }
        if (value == "mythic") { outTier = ItemTier::Mythic; return true; }
        if (value == "legendary") { outTier = ItemTier::Legendary; return true; }
        return false;
    }

    bool BuildDevItem(ItemTier tier, const std::string& typeToken, Item& outItem) {
        std::string type = ToLower(typeToken);

        if (type == "healthpotion" || type == "health_potion" || type == "potion") {
            outItem = Item{};
            outItem.category = ItemCategory::Consumable;
            outItem.tier = ItemTier::Common;
            outItem.stackable = true;
            outItem.quantity = 1;
            outItem.name = "Health Potion";
            outItem.consumableType = ConsumableType::HealthPotion;
            return true;
        }
        if (type == "armorpotion" || type == "armor_potion") {
            outItem = Item{};
            outItem.category = ItemCategory::Consumable;
            outItem.tier = ItemTier::Common;
            outItem.stackable = true;
            outItem.quantity = 1;
            outItem.name = "Armor Potion";
            outItem.consumableType = ConsumableType::ArmorPotion;
            return true;
        }
        if (type == "antidote") {
            outItem = Item{};
            outItem.category = ItemCategory::Consumable;
            outItem.tier = ItemTier::Common;
            outItem.stackable = true;
            outItem.quantity = 1;
            outItem.name = "Antidote";
            outItem.consumableType = ConsumableType::Antidote;
            return true;
        }
        if (type == "bandage") {
            outItem = Item{};
            outItem.category = ItemCategory::Consumable;
            outItem.tier = ItemTier::Common;
            outItem.stackable = true;
            outItem.quantity = 1;
            outItem.name = "Bandage";
            outItem.consumableType = ConsumableType::Bandage;
            return true;
        }

        outItem = Item{};
        outItem.category = ItemCategory::Equipment;
        outItem.tier = tier;
        outItem.itemLevel = std::max(1, dungeonLevel);
        outItem.basePower = outItem.itemLevel * 3;
        outItem.rarityMultiplier = RollRarityMultiplier(tier);
        outItem.itemPower = static_cast<float>(outItem.basePower) * outItem.rarityMultiplier;

        if (type == "sword" || type == "weapon" || type == "weapon_sword") {
            outItem.slot = EquipSlot::Weapon;
            outItem.weaponType = WeaponType::Sword;
            outItem.name = std::string(ItemTierLabel(tier)) + " Sword";
            return true;
        }
        if (type == "bow" || type == "weapon_bow") {
            outItem.slot = EquipSlot::Weapon;
            outItem.weaponType = WeaponType::Bow;
            outItem.name = std::string(ItemTierLabel(tier)) + " Bow";
            return true;
        }
        if (type == "staff" || type == "weapon_staff") {
            outItem.slot = EquipSlot::Weapon;
            outItem.weaponType = WeaponType::Staff;
            outItem.name = std::string(ItemTierLabel(tier)) + " Staff";
            return true;
        }
        if (type == "head" || type == "helm") {
            outItem.slot = EquipSlot::Head;
            outItem.name = std::string(ItemTierLabel(tier)) + " Helm";
            return true;
        }
        if (type == "top" || type == "armor") {
            outItem.slot = EquipSlot::Top;
            outItem.name = std::string(ItemTierLabel(tier)) + " Armor";
            return true;
        }
        if (type == "feet" || type == "boots") {
            outItem.slot = EquipSlot::Feet;
            outItem.name = std::string(ItemTierLabel(tier)) + " Boots";
            return true;
        }
        if (type == "hands" || type == "gauntlets") {
            outItem.slot = EquipSlot::Hands;
            outItem.name = std::string(ItemTierLabel(tier)) + " Gauntlets";
            return true;
        }
        if (type == "ring") {
            outItem.slot = EquipSlot::Ring1;
            outItem.name = std::string(ItemTierLabel(tier)) + " Ring";
            return true;
        }
        if (type == "necklace" || type == "amulet") {
            outItem.slot = EquipSlot::Necklace;
            outItem.name = std::string(ItemTierLabel(tier)) + " Amulet";
            return true;
        }
        if (type == "shield") {
            outItem.slot = EquipSlot::Shield;
            outItem.name = std::string(ItemTierLabel(tier)) + " Shield";
            return true;
        }

        return false;
    }

    bool ParseMonsterToken(const std::string& token, EnemyArchetype& outArchetype, int& outIconRow, int& outIconCol) const {
        std::string value = ToLower(token);
        if (value == "goblinwarrior" || value == "goblin_warrior") { outArchetype = EnemyArchetype::GoblinWarrior; outIconRow = 0; outIconCol = 0; return true; }
        if (value == "goblinranger" || value == "goblin_ranger") { outArchetype = EnemyArchetype::GoblinRanger; outIconRow = 0; outIconCol = 1; return true; }
        if (value == "goblinwizard" || value == "goblin_wizard") { outArchetype = EnemyArchetype::GoblinWizard; outIconRow = 0; outIconCol = 2; return true; }
        if (value == "undeadwarrior" || value == "undead_warrior") { outArchetype = EnemyArchetype::UndeadWarrior; outIconRow = 1; outIconCol = 0; return true; }
        if (value == "undeadranger" || value == "undead_ranger") { outArchetype = EnemyArchetype::UndeadRanger; outIconRow = 1; outIconCol = 1; return true; }
        if (value == "undeadwizard" || value == "undead_wizard") { outArchetype = EnemyArchetype::UndeadWizard; outIconRow = 1; outIconCol = 2; return true; }
        if (value == "rat" || value == "rats") { outArchetype = EnemyArchetype::Rat; outIconRow = 2; outIconCol = 0; return true; }
        if (value == "wolf" || value == "wolves") { outArchetype = EnemyArchetype::Wolf; outIconRow = 2; outIconCol = 1; return true; }
        if (value == "bat" || value == "bats") { outArchetype = EnemyArchetype::Bat; outIconRow = 2; outIconCol = 2; return true; }
        return false;
    }

    bool SpawnDevEnemy(EnemyArchetype archetype, int iconRow, int iconCol) {
        int spawnX = -1;
        int spawnY = -1;
        for (int attempts = 0; attempts < 200; ++attempts) {
            int x = RandomInt(1, kMapWidth - 2);
            int y = RandomInt(1, kMapHeight - 2);
            if (!IsWalkable(x, y)) continue;
            if (x == player.x && y == player.y) continue;
            if (IsEnemyAt(x, y)) continue;
            spawnX = x;
            spawnY = y;
            break;
        }
        if (spawnX < 0 || spawnY < 0) return false;

        Enemy enemy;
        enemy.x = spawnX;
        enemy.y = spawnY;
        enemy.hp = RandomInt(3 + dungeonLevel, 6 + dungeonLevel * 2);
        enemy.maxHp = enemy.hp;
        enemy.atk = RandomInt(2 + dungeonLevel / 2, 4 + dungeonLevel);
        enemy.alive = true;
        enemy.archetype = archetype;
        enemy.staffCharge = 5;
        enemy.isFlying = (archetype == EnemyArchetype::Bat);

        if (enemyAtlasLoaded) {
            int cols = std::max(1, enemyAtlas.width / kItemIconSize);
            int rows = std::max(1, enemyAtlas.height / kItemIconSize);
            int clampedRow = std::min(iconRow, rows - 1);
            int clampedCol = std::min(iconCol, cols - 1);
            enemy.iconVariant = clampedRow * cols + clampedCol;
        } else {
            enemy.iconVariant = iconCol;
        }

        enemies.push_back(enemy);
        return true;
    }

    void ExecuteDevCommand(const std::string& rawInput) {
        std::vector<std::string> tokens = TokenizeCommand(rawInput);
        if (tokens.empty()) return;

        std::string command = ToLower(tokens[0]);

        if (command == "help") {
            AddDevConsoleLine("Commands:");
            AddDevConsoleLine("  additem [rarity] [type] [quantity]");
            AddDevConsoleLine("  addgold [amount]");
            AddDevConsoleLine("  removegold [amount]");
            AddDevConsoleLine("  clear floor");
            AddDevConsoleLine("  spawn [monster] [quantity]");
            AddDevConsoleLine("  spawn gold [quantity]");
            AddDevConsoleLine("  poison player");
            AddDevConsoleLine("  bleed player");
            AddDevConsoleLine("Rarities: common uncommon rare mythic legendary");
            AddDevConsoleLine("Types: sword bow staff head top feet hands ring necklace shield");
            AddDevConsoleLine("       healthpotion armorpotion antidote bandage");
            AddDevConsoleLine("Monsters: goblinWarrior goblinRanger goblinWizard");
            AddDevConsoleLine("          undeadWarrior undeadRanger undeadWizard rat wolf bat");
            return;
        }

        if (command == "addgold") {
            if (tokens.size() < 2) {
                AddDevConsoleLine("Usage: addgold [amount]");
                return;
            }

            int amount = 0;
            try {
                amount = std::stoi(tokens[1]);
            } catch (...) {
                AddDevConsoleLine("Amount must be a number.");
                return;
            }

            if (amount <= 0) {
                AddDevConsoleLine("Amount must be greater than 0.");
                return;
            }

            player.gold += amount;
            AddDevConsoleLine("Added " + std::to_string(amount) + " gold.");
            AddLog("[DEV] Added " + std::to_string(amount) + " gold.");
            return;
        }

        if (command == "removegold") {
            if (tokens.size() < 2) {
                AddDevConsoleLine("Usage: removegold [amount]");
                return;
            }

            int amount = 0;
            try {
                amount = std::stoi(tokens[1]);
            } catch (...) {
                AddDevConsoleLine("Amount must be a number.");
                return;
            }

            if (amount <= 0) {
                AddDevConsoleLine("Amount must be greater than 0.");
                return;
            }

            int removed = std::min(player.gold, amount);
            player.gold -= removed;
            AddDevConsoleLine("Removed " + std::to_string(removed) + " gold.");
            AddLog("[DEV] Removed " + std::to_string(removed) + " gold.");
            return;
        }

        if (command == "additem") {
            std::optional<ItemTier> parsedTier;
            std::string typeToken;
            int quantity = 1;

            for (size_t i = 1; i < tokens.size(); ++i) {
                ItemTier tierCandidate = ItemTier::Common;
                if (!parsedTier.has_value() && ParseItemTierToken(tokens[i], tierCandidate)) {
                    parsedTier = tierCandidate;
                    continue;
                }

                try {
                    int qtyCandidate = std::stoi(tokens[i]);
                    quantity = std::max(1, qtyCandidate);
                    continue;
                } catch (...) {
                }

                if (typeToken.empty()) {
                    typeToken = tokens[i];
                }
            }

            ItemTier tier = parsedTier.has_value() ? parsedTier.value() : RollItemTier();

            if (typeToken.empty()) {
                const std::array<const char*, 14> randomTypes = {
                    "sword", "bow", "staff", "head", "top", "feet",
                    "hands", "ring", "necklace", "shield", "healthpotion", "armorpotion", "antidote", "bandage"
                };
                typeToken = randomTypes[RandomInt(0, static_cast<int>(randomTypes.size()) - 1)];
            }

            Item item;
            if (!BuildDevItem(tier, typeToken, item)) {
                AddDevConsoleLine("Unknown item type.");
                return;
            }

            for (int i = 0; i < quantity; ++i) {
                AddItemToInventory(item);
            }

            AddDevConsoleLine("Added " + std::to_string(quantity) + "x " + item.name);
            AddLog("[DEV] Added " + std::to_string(quantity) + "x " + item.name);
            return;
        }

        if (command == "clear" && tokens.size() >= 2 && ToLower(tokens[1]) == "floor") {
            int removed = static_cast<int>(enemies.size());
            enemies.clear();
            TryOpenMerchantRoom();
            AddDevConsoleLine("Cleared floor: removed " + std::to_string(removed) + " enemies.");
            AddLog("[DEV] Floor cleared.");
            return;
        }

        if (command == "spawn") {
            if (tokens.size() < 2) {
                AddDevConsoleLine("Usage: spawn [monster] [quantity]");
                return;
            }

            if (ToLower(tokens[1]) == "gold") {
                int quantity = 1;
                if (tokens.size() >= 3) {
                    try {
                        quantity = std::max(1, std::stoi(tokens[2]));
                    } catch (...) {
                        AddDevConsoleLine("Quantity must be a number.");
                        return;
                    }
                }

                int spawned = 0;
                for (int i = 0; i < quantity; ++i) {
                    if (TrySpawnRandomGoldStack()) {
                        spawned += 1;
                    }
                }

                AddDevConsoleLine("Spawned " + std::to_string(spawned) + " gold stack(s).");
                AddLog("[DEV] Spawned " + std::to_string(spawned) + " gold stack(s).");
                return;
            }

            if (tokens.size() < 3) {
                AddDevConsoleLine("Usage: spawn [monster] [quantity]");
                AddDevConsoleLine("Usage: spawn gold [quantity]");
                return;
            }

            EnemyArchetype archetype = EnemyArchetype::GoblinWarrior;
            int iconRow = 0;
            int iconCol = 0;
            if (!ParseMonsterToken(tokens[1], archetype, iconRow, iconCol)) {
                AddDevConsoleLine("Unknown monster.");
                return;
            }

            int quantity = 1;
            try {
                quantity = std::max(1, std::stoi(tokens[2]));
            } catch (...) {
                AddDevConsoleLine("Quantity must be a number.");
                return;
            }

            int spawned = 0;
            for (int i = 0; i < quantity; ++i) {
                if (SpawnDevEnemy(archetype, iconRow, iconCol)) {
                    spawned += 1;
                }
            }

            AddDevConsoleLine("Spawned " + std::to_string(spawned) + " " + EnemyName(archetype) + "(s).");
            AddLog("[DEV] Spawned " + std::to_string(spawned) + " " + EnemyName(archetype) + "(s).");
            return;
        }

        if (command == "poison" && tokens.size() >= 2 && ToLower(tokens[1]) == "player") {
            bool wasPoisoned = player.poisoned;
            player.poisoned = true;
            if (!wasPoisoned) {
                TriggerPoisonAfflictFlash();
            }
            AddDevConsoleLine("Player is now Poisoned.");
            AddLog("[DEV] Player poisoned.");
            return;
        }

        if (command == "bleed" && tokens.size() >= 2 && ToLower(tokens[1]) == "player") {
            bool wasBleeding = player.bleeding;
            player.bleeding = true;
            if (!wasBleeding) {
                TriggerBleedAfflictFlash();
            }
            AddDevConsoleLine("Player is now Bleeding.");
            AddLog("[DEV] Player bleeding.");
            return;
        }

        AddDevConsoleLine("Unknown command.");
    }

    void HandleDevConsoleInput() {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {
                devConsoleInput.push_back(static_cast<char>(key));
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && !devConsoleInput.empty()) {
            devConsoleInput.pop_back();
        }

        if (IsKeyPressed(KEY_ENTER)) {
            if (!devConsoleInput.empty()) {
                AddDevConsoleLine("> " + devConsoleInput);
                ExecuteDevCommand(devConsoleInput);
                devConsoleInput.clear();
            }
        }
    }

    void DrawDevConsole() const {
        const int panelX = 18;
        const int panelY = kScreenHeight - 220;
        const int panelW = kScreenWidth - 36;
        const int panelH = 202;

        DrawRectangle(panelX, panelY, panelW, panelH, Color{8, 10, 16, 228});
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{90, 110, 145, 255});
        DrawText("DEV CONSOLE (` to toggle)", panelX + 12, panelY + 10, 24, RAYWHITE);

        const int lineHeight = 22;
        const int logTop = panelY + 42;
        const int promptTop = panelY + panelH - 42;
        const int maxVisibleLines = std::max(1, (promptTop - logTop - 4) / lineHeight);
        int startLine = 0;
        if (static_cast<int>(devConsoleHistory.size()) > maxVisibleLines) {
            startLine = static_cast<int>(devConsoleHistory.size()) - maxVisibleLines;
        }

        int y = logTop;
        for (int i = startLine; i < static_cast<int>(devConsoleHistory.size()); ++i) {
            const std::string& line = devConsoleHistory[i];
            DrawText(line.c_str(), panelX + 12, y, 20, LIGHTGRAY);
            y += lineHeight;
        }

        std::string prompt = "> " + devConsoleInput;
        DrawRectangle(panelX + 10, panelY + panelH - 42, panelW - 20, 30, Color{20, 24, 34, 240});
        DrawText(prompt.c_str(), panelX + 16, panelY + panelH - 38, 22, Color{200, 220, 255, 255});
    }

    int RandomInt(int minValue, int maxValue) {
        std::uniform_int_distribution<int> dist(minValue, maxValue);
        return dist(rng);
    }

    ItemTier RollItemTier() {
        int roll = RandomInt(1, 1000);
        if (roll <= 600) return ItemTier::Common;
        if (roll <= 900) return ItemTier::Uncommon;
        if (roll <= 960) return ItemTier::Rare;
        if (roll <= 990) return ItemTier::Mythic;
        return ItemTier::Legendary;
    }

    float RollRarityMultiplier(ItemTier tier) {
        auto rollRange = [&](float low, float high) {
            int low10 = static_cast<int>(std::round(low * 10.0f));
            int high10 = static_cast<int>(std::round(high * 10.0f));
            int roll10 = RandomInt(low10, high10);
            return static_cast<float>(roll10) / 10.0f;
        };

        switch (tier) {
        case ItemTier::Common: return rollRange(1.0f, 1.1f);
        case ItemTier::Uncommon: return rollRange(1.2f, 1.3f);
        case ItemTier::Rare: return rollRange(1.4f, 1.5f);
        case ItemTier::Mythic: return rollRange(1.6f, 1.7f);
        case ItemTier::Legendary: return rollRange(1.8f, 2.0f);
        default: return 1.0f;
        }
    }

    int TierScalar(ItemTier tier) const {
        switch (tier) {
        case ItemTier::Common: return 1;
        case ItemTier::Uncommon: return 2;
        case ItemTier::Rare: return 3;
        case ItemTier::Mythic: return 5;
        case ItemTier::Legendary: return 7;
        default: return 1;
        }
    }

    int ComputeSellValue(const Item& item) const {
        float rawValue = (item.itemPower * static_cast<float>(dungeonLevel)) / 2.0f;
        return std::max(0, static_cast<int>(std::floor(rawValue)));
    }

    int ComputeBuyValue(const Item& item) const {
        return std::max(1, ComputeSellValue(item) * 3);
    }

    int ComputeRerollCost(const Item& item) const {
        return std::max(1, ComputeSellValue(item));
    }

    void ApplyRandomEquipmentRoll(Item& item) {
        if (item.category != ItemCategory::Equipment) return;

        item.hpBonus = 0;
        item.armorBonus = 0;
        item.strBonus = 0;
        item.dexBonus = 0;
        item.intBonus = 0;

        item.itemLevel = std::max(1, item.itemLevel);
        item.basePower = std::max(3, item.basePower);
        item.rarityMultiplier = RollRarityMultiplier(item.tier);
        item.itemPower = static_cast<float>(item.basePower) * item.rarityMultiplier;

        int primaryStat = (item.basePower + 1) / 2;
        int secondaryStat = item.basePower / 4;

        struct AffixRoll {
            int primaryType;
            int secondaryType;
            const char* suffix;
        };

        const std::array<AffixRoll, 9> affixes = {{
            {0, 0, "of the Titan"},
            {0, 1, "of the Duelist"},
            {0, 2, "of the Paladin"},
            {1, 1, "of the Ranger"},
            {1, 0, "of the Slayer"},
            {1, 2, "of the Stalker"},
            {2, 2, "of the Archmage"},
            {2, 0, "of the Spellblade"},
            {2, 1, "of the Mystic"}
        }};

        const AffixRoll& affix = affixes[RandomInt(0, static_cast<int>(affixes.size()) - 1)];
        item.primaryStatType = affix.primaryType;

        auto applyBonus = [&](int statType, int amount) {
            if (amount <= 0) return;
            if (statType == 0) item.strBonus += amount;
            if (statType == 1) item.dexBonus += amount;
            if (statType == 2) item.intBonus += amount;
        };

        applyBonus(affix.primaryType, primaryStat);
        applyBonus(affix.secondaryType, secondaryStat);

        std::string baseItem;
        if (item.slot == EquipSlot::Weapon) {
            if (item.weaponType == WeaponType::Sword) baseItem = "Sword";
            if (item.weaponType == WeaponType::Bow) baseItem = "Bow";
            if (item.weaponType == WeaponType::Staff) baseItem = "Staff";
        } else {
            baseItem = BaseItemNameForSlot(item.slot);
        }

        std::string material = MaterialPrefixForTier(item.tier);
        item.name = material + " " + baseItem + " " + affix.suffix;
    }

    Item CreateRandomConsumableDrop() {
        Item item;
        item.category = ItemCategory::Consumable;
        item.tier = ItemTier::Common;
        item.itemLevel = std::max(1, dungeonLevel);
        item.basePower = 1;
        item.rarityMultiplier = 1.0f;
        item.itemPower = 1.0f;
        item.stackable = true;
        item.quantity = 1;

        int pick = RandomInt(0, 3);
        if (pick == 0) {
            item.name = "Health Potion";
            item.consumableType = ConsumableType::HealthPotion;
        } else if (pick == 1) {
            item.name = "Armor Potion";
            item.consumableType = ConsumableType::ArmorPotion;
        } else if (pick == 2) {
            item.name = "Antidote";
            item.consumableType = ConsumableType::Antidote;
        } else {
            item.name = "Bandage";
            item.consumableType = ConsumableType::Bandage;
        }

        int bonusQty = std::max(0, TierScalar(ItemTier::Common) - 1);
        if (bonusQty > 0 && RandomInt(0, 1) == 1) {
            item.quantity += 1;
        }
        return item;
    }

    Item CreateRandomEquipmentDrop(ItemTier tier) {
        Item item;
        item.category = ItemCategory::Equipment;
        item.tier = tier;

        item.itemLevel = std::max(1, dungeonLevel);
        item.basePower = item.itemLevel * 3;

        std::array<EquipSlot, 8> slots = {
            EquipSlot::Head, EquipSlot::Top, EquipSlot::Feet, EquipSlot::Hands,
            EquipSlot::Ring1, EquipSlot::Necklace, EquipSlot::Weapon, EquipSlot::Shield
        };
        item.slot = slots[RandomInt(0, static_cast<int>(slots.size()) - 1)];

        if (item.slot == EquipSlot::Weapon) {
            std::array<WeaponType, 3> weaponTypes = {WeaponType::Sword, WeaponType::Bow, WeaponType::Staff};
            item.weaponType = weaponTypes[RandomInt(0, static_cast<int>(weaponTypes.size()) - 1)];
        }
        ApplyRandomEquipmentRoll(item);
        return item;
    }

    void MaybeDropItemFromEnemy() {
        int dropRoll = RandomInt(1, 100);
        if (dropRoll > 50) {
            return;
        }

        ItemTier tier = RollItemTier();
        Item drop;
        if (RandomInt(0, 99) < 55) {
            drop = CreateRandomEquipmentDrop(tier);
        } else {
            drop = CreateRandomConsumableDrop();
        }

        AddItemToInventory(drop);
        AddLog("Loot: " + drop.name + " (" + ItemTierLabel(drop.tier) + ")");
    }

    bool IsMerchantAt(int x, int y) const {
        return merchantPresent && x == merchantX && y == merchantY;
    }

    bool IsInsideMerchantChamber(int x, int y) const {
        if (!merchantPresent) return false;
        return x >= (merchantX - 1) && x <= (merchantX + 1) && y >= (merchantY - 1) && y <= (merchantY + 1);
    }

    bool IsMerchantChamberLockedTile(int x, int y) const {
        if (!merchantPresent || merchantRoomOpened) return false;
        if (IsMerchantAt(x, y)) return false;
        return IsInsideMerchantChamber(x, y);
    }

    bool CanInteractWithMerchant() const {
        if (!merchantPresent) return false;
        int dist = std::abs(player.x - merchantX) + std::abs(player.y - merchantY);
        return dist <= 1;
    }

    bool AreAllFloorMonstersDefeated() const {
        for (const Enemy& enemy : enemies) {
            if (enemy.alive) return false;
        }
        return true;
    }

    void TryOpenMerchantRoom() {
        if (!merchantPresent || merchantRoomOpened) return;
        if (merchantDoorX < 0 || merchantDoorY < 0) return;
        if (!AreAllFloorMonstersDefeated()) return;

        tiles[Index(merchantDoorX, merchantDoorY)] = TileType::Floor;
        merchantRoomOpened = true;
        PlaySfx(sfxStairs, sfxStairsLoaded);
        AddLog("You hear stone grinding... the Merchant room opens.");
    }

    const char* MerchantTabLabel(MerchantTab tab) const {
        switch (tab) {
        case MerchantTab::Buy: return "Buy";
        case MerchantTab::Reroll: return "Reroll";
        case MerchantTab::Sell: return "Sell";
        default: return "Buy";
        }
    }

    std::vector<int> MerchantInventoryIndices(bool equipmentOnly) const {
        std::vector<int> indices;
        for (int i = 0; i < static_cast<int>(player.inventory.size()); ++i) {
            if (equipmentOnly && player.inventory[i].category != ItemCategory::Equipment) continue;
            indices.push_back(i);
        }
        return indices;
    }

    void GenerateMerchantStock() {
        merchantStock.clear();
        int stockCount = 5 + RandomInt(0, 2);
        for (int i = 0; i < stockCount; ++i) {
            Item stockItem;
            if (RandomInt(0, 99) < 70) {
                stockItem = CreateRandomEquipmentDrop(RollItemTier());
            } else {
                stockItem = CreateRandomConsumableDrop();
            }
            merchantStock.push_back(stockItem);
        }
    }

    bool TryPlaceMerchantChamber(int& outCenterX, int& outCenterY, int& outDoorX, int& outDoorY) {
        struct Candidate {
            int centerX;
            int centerY;
            int doorX;
            int doorY;
        };

        std::vector<Candidate> candidates;
        const int dirs[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};

        auto isSolidTile = [&](int x, int y) {
            if (!InBounds(x, y)) return false;
            TileType t = tiles[Index(x, y)];
            return t == TileType::Wall || t == TileType::Outside;
        };

        for (int y = 1; y < kMapHeight - 1; ++y) {
            for (int x = 1; x < kMapWidth - 1; ++x) {
                if (tiles[Index(x, y)] != TileType::Floor) continue;

                for (const auto& dir : dirs) {
                    int dx = dir[0];
                    int dy = dir[1];

                    int centerX = x + dx * 3;
                    int centerY = y + dy * 3;
                    int doorX = x + dx;
                    int doorY = y + dy;

                    if (!InBounds(centerX - 2, centerY - 2) || !InBounds(centerX + 2, centerY + 2)) continue;
                    if (!isSolidTile(doorX, doorY)) continue;

                    bool chamberIsClear = true;
                    for (int ry = centerY - 1; ry <= centerY + 1 && chamberIsClear; ++ry) {
                        for (int rx = centerX - 1; rx <= centerX + 1; ++rx) {
                            if (!isSolidTile(rx, ry)) {
                                chamberIsClear = false;
                                break;
                            }
                        }
                    }

                    if (!chamberIsClear) continue;

                    bool outerRingClear = true;
                    for (int ry = centerY - 2; ry <= centerY + 2 && outerRingClear; ++ry) {
                        for (int rx = centerX - 2; rx <= centerX + 2; ++rx) {
                            bool isRing = (std::max(std::abs(rx - centerX), std::abs(ry - centerY)) == 2);
                            if (!isRing) continue;
                            if (rx == x && ry == y) continue; // existing corridor connector tile
                            if (!isSolidTile(rx, ry)) {
                                outerRingClear = false;
                                break;
                            }
                        }
                    }

                    if (!outerRingClear) continue;
                    candidates.push_back(Candidate{centerX, centerY, doorX, doorY});
                }
            }
        }

        if (candidates.empty()) return false;

        const Candidate& picked = candidates[RandomInt(0, static_cast<int>(candidates.size()) - 1)];
        outCenterX = picked.centerX;
        outCenterY = picked.centerY;
        outDoorX = picked.doorX;
        outDoorY = picked.doorY;
        return true;
    }

    void SetupMerchantForFloor() {
        merchantPresent = false;
        merchantOpen = false;
        merchantX = -1;
        merchantY = -1;
        merchantDoorX = -1;
        merchantDoorY = -1;
        merchantRoomOpened = false;
        merchantIconVariant = 0;
        merchantStock.clear();

        if (dungeonLevel != nextMerchantFloor) return;

        int chamberCenterX = -1;
        int chamberCenterY = -1;
        int chamberDoorX = -1;
        int chamberDoorY = -1;
        if (!TryPlaceMerchantChamber(chamberCenterX, chamberCenterY, chamberDoorX, chamberDoorY)) {
            return;
        }

        for (int y = chamberCenterY - 1; y <= chamberCenterY + 1; ++y) {
            for (int x = chamberCenterX - 1; x <= chamberCenterX + 1; ++x) {
                tiles[Index(x, y)] = TileType::Floor;
            }
        }

        for (int y = chamberCenterY - 2; y <= chamberCenterY + 2; ++y) {
            for (int x = chamberCenterX - 2; x <= chamberCenterX + 2; ++x) {
                bool isRing = (std::max(std::abs(x - chamberCenterX), std::abs(y - chamberCenterY)) == 2);
                if (!isRing) continue;
                if (x == chamberDoorX && y == chamberDoorY) continue;
                if (!InBounds(x, y)) continue;
                tiles[Index(x, y)] = TileType::Wall;
            }
        }

        tiles[Index(chamberDoorX, chamberDoorY)] = TileType::Wall;

        merchantX = chamberCenterX;
        merchantY = chamberCenterY;
        merchantDoorX = chamberDoorX;
        merchantDoorY = chamberDoorY;
        merchantRoomOpened = false;

        if (npcAtlasLoaded) {
            int cols = std::max(1, npcAtlas.width / kItemIconSize);
            int rows = std::max(1, npcAtlas.height / kItemIconSize);
            int tileCount = cols * rows;
            merchantIconVariant = (tileCount > 0) ? RandomInt(0, tileCount - 1) : 0;
        } else {
            merchantIconVariant = 0;
        }

        merchantPresent = true;
        merchantTab = MerchantTab::Buy;
        merchantSelection = 0;

        GenerateMerchantStock();
        nextMerchantFloor += RandomInt(3, 5);
        AddLog("A Merchant has set up shop on this floor.");
    }

    bool IsOccupiedByEnemy(int x, int y, int* enemyIndex = nullptr) {
        for (int i = 0; i < static_cast<int>(enemies.size()); ++i) {
            if (enemies[i].alive && enemies[i].x == x && enemies[i].y == y) {
                if (enemyIndex != nullptr) *enemyIndex = i;
                return true;
            }
        }
        return false;
    }

    bool RoomsOverlap(const Room& a, const Room& b) const {
        return a.x <= (b.x + b.w) && (a.x + a.w) >= b.x && a.y <= (b.y + b.h) && (a.y + a.h) >= b.y;
    }

    std::pair<int, int> RoomCenter(const Room& room) const {
        return {room.x + room.w / 2, room.y + room.h / 2};
    }

    bool IsInsideRoom(int x, int y, const Room& room) const {
        return x >= room.x && x < (room.x + room.w) && y >= room.y && y < (room.y + room.h);
    }

    bool IsInsideAnyRoom(int x, int y) const {
        for (const Room& room : rooms) {
            if (IsInsideRoom(x, y, room)) return true;
        }
        return false;
    }

    void CarveRoom(const Room& room) {
        for (int y = room.y; y < room.y + room.h; ++y) {
            for (int x = room.x; x < room.x + room.w; ++x) {
                tiles[Index(x, y)] = TileType::Floor;
            }
        }
    }

    void CarveCorridor(int x1, int y1, int x2, int y2) {
        int x = x1;
        int y = y1;

        while (x != x2) {
            tiles[Index(x, y)] = TileType::Floor;
            x += (x2 > x) ? 1 : -1;
        }
        while (y != y2) {
            tiles[Index(x, y)] = TileType::Floor;
            y += (y2 > y) ? 1 : -1;
        }
        tiles[Index(x2, y2)] = TileType::Floor;
    }

    void SpawnEnemies() {
        enemies.clear();
        PassiveRuntimeEffects passiveEffects = ComputePassiveEffects();

        for (int i = 1; i < static_cast<int>(rooms.size()); ++i) {
            const Room& room = rooms[i];
            int count = RandomInt(1, 2 + dungeonLevel / 2);
            count = std::max(0, static_cast<int>(std::round(static_cast<float>(count) * std::max(0.20f, passiveEffects.enemySpawnMult))));
            for (int j = 0; j < count; ++j) {
                int attempts = 0;
                while (attempts < 25) {
                    int ex = RandomInt(room.x, room.x + room.w - 1);
                    int ey = RandomInt(room.y, room.y + room.h - 1);
                    if ((ex == player.x && ey == player.y) || IsOccupiedByEnemy(ex, ey) || IsMerchantAt(ex, ey) || IsAnyInteractableAt(ex, ey)) {
                        ++attempts;
                        continue;
                    }

                    int rolledHp = RandomInt(3 + dungeonLevel, 6 + dungeonLevel * 2);
                    int rolledAtk = RandomInt(2 + dungeonLevel / 2, 4 + dungeonLevel);

                    Enemy enemy;
                    enemy.x = ex;
                    enemy.y = ey;
                    enemy.hp = rolledHp;
                    enemy.maxHp = enemy.hp;   // NEW
                    enemy.atk = rolledAtk;
                    enemy.alive = true;
                    enemy.staffCharge = 5;

                    int iconRow = 0;
                    int iconCol = 0;

                    if (dungeonLevel <= 10) {
                        int poolRoll = RandomInt(0, 5);
                        if (poolRoll <= 2) {
                            iconRow = 0;
                            iconCol = poolRoll;
                            if (iconCol == 0) enemy.archetype = EnemyArchetype::GoblinWarrior;
                            if (iconCol == 1) enemy.archetype = EnemyArchetype::GoblinRanger;
                            if (iconCol == 2) enemy.archetype = EnemyArchetype::GoblinWizard;
                        } else {
                            iconRow = 2;
                            iconCol = poolRoll - 3;
                            if (iconCol == 0) enemy.archetype = EnemyArchetype::Rat;
                            if (iconCol == 1) enemy.archetype = EnemyArchetype::Wolf;
                            if (iconCol == 2) enemy.archetype = EnemyArchetype::Bat;
                        }
                    } else if (dungeonLevel <= 20) {
                        int poolRoll = RandomInt(0, 5);
                        if (poolRoll <= 2) {
                            iconRow = 1;
                            iconCol = poolRoll;
                            if (iconCol == 0) enemy.archetype = EnemyArchetype::UndeadWarrior;
                            if (iconCol == 1) enemy.archetype = EnemyArchetype::UndeadRanger;
                            if (iconCol == 2) enemy.archetype = EnemyArchetype::UndeadWizard;
                        } else {
                            iconRow = 2;
                            iconCol = poolRoll - 3;
                            if (iconCol == 0) enemy.archetype = EnemyArchetype::Rat;
                            if (iconCol == 1) enemy.archetype = EnemyArchetype::Wolf;
                            if (iconCol == 2) enemy.archetype = EnemyArchetype::Bat;
                        }
                    } else {
                        int typeRoll = RandomInt(0, 2);
                        iconRow = 2;
                        iconCol = typeRoll;
                        if (typeRoll == 0) enemy.archetype = EnemyArchetype::Rat;
                        if (typeRoll == 1) enemy.archetype = EnemyArchetype::Wolf;
                        if (typeRoll == 2) enemy.archetype = EnemyArchetype::Bat;
                    }

                    enemy.isFlying = (enemy.archetype == EnemyArchetype::Bat);

                    if (enemyAtlasLoaded) {
                        int cols = std::max(1, enemyAtlas.width / kItemIconSize);
                        int maxRows = std::max(1, enemyAtlas.height / kItemIconSize);
                        int clampedRow = std::min(iconRow, maxRows - 1);
                        int clampedCol = std::min(iconCol, cols - 1);
                        enemy.iconVariant = clampedRow * cols + clampedCol;
                    } else {
                        enemy.iconVariant = iconCol;
                    }

                    enemies.push_back(enemy);
                    break;
                }
            }
        }
    }

    bool IsGoldAt(int x, int y) const {
        for (const GoldStack& stack : goldStacks) {
            if (stack.x == x && stack.y == y) return true;
        }
        return false;
    }

    GoldStack RollGoldStackAt(int x, int y) {
        int depthBonus = std::min(40, (dungeonLevel - 1) * 2);
        int smallChance = std::max(5, 60 - depthBonus);
        int mediumChance = 30;
        int largeChance = 10 + depthBonus;
        int chanceSum = smallChance + mediumChance + largeChance;

        int roll = RandomInt(1, chanceSum);

        GoldStack stack;
        stack.x = x;
        stack.y = y;
        if (roll <= smallChance) {
            stack.type = GoldStackType::Small;
            stack.amount = RandomInt(1, 10);
        } else if (roll <= smallChance + mediumChance) {
            stack.type = GoldStackType::Medium;
            stack.amount = RandomInt(11, 50);
        } else {
            stack.type = GoldStackType::Large;
            stack.amount = RandomInt(51, 100);
        }

        PassiveRuntimeEffects passiveEffects = ComputePassiveEffects();
        stack.amount = std::max(1, static_cast<int>(std::round(static_cast<float>(stack.amount) * std::max(0.05f, passiveEffects.goldDropMult))));
        if (stack.amount >= 51) stack.type = GoldStackType::Large;
        else if (stack.amount >= 11) stack.type = GoldStackType::Medium;
        else stack.type = GoldStackType::Small;

        return stack;
    }

    bool TrySpawnRandomGoldStack() {
        int stairsX = -1;
        int stairsY = -1;
        if (!rooms.empty()) {
            auto stairsPos = RoomCenter(rooms.back());
            stairsX = stairsPos.first;
            stairsY = stairsPos.second;
        }

        for (int attempts = 0; attempts < 120; ++attempts) {
            int gx = RandomInt(1, kMapWidth - 2);
            int gy = RandomInt(1, kMapHeight - 2);
            if (!IsWalkable(gx, gy)) continue;
            if (gx == player.x && gy == player.y) continue;
            if (gx == stairsX && gy == stairsY) continue;
            if (IsEnemyAt(gx, gy)) continue;
            if (IsGoldAt(gx, gy)) continue;
            if (IsMerchantAt(gx, gy)) continue;
            if (IsAnyInteractableAt(gx, gy)) continue;

            goldStacks.push_back(RollGoldStackAt(gx, gy));
            return true;
        }

        return false;
    }

    void SpawnGoldStacks() {
        goldStacks.clear();

        int minStacks = 2;
        int maxStacks = 4 + dungeonLevel / 5;
        int stackCount = RandomInt(minStacks, std::max(minStacks, maxStacks));

        for (int s = 0; s < stackCount; ++s) {
            if (!TrySpawnRandomGoldStack()) break;
        }
    }

    void SpawnDecorations() {
        walkableDecorations.clear();
        solidDecorations.clear();

        int walkableVariantCount = 0;
        if (walkablesAtlasLoaded) {
            int cols = walkablesAtlas.width / kItemIconSize;
            int rows = walkablesAtlas.height / kItemIconSize;
            walkableVariantCount = std::max(0, cols * rows);
        }

        int solidVariantCount = 0;
        if (solidsAtlasLoaded) {
            int cols = solidsAtlas.width / kItemIconSize;
            int rows = solidsAtlas.height / kItemIconSize;
            solidVariantCount = std::max(0, cols * rows);
        }

        for (int y = 1; y < kMapHeight - 1; ++y) {
            for (int x = 1; x < kMapWidth - 1; ++x) {
                if (tiles[Index(x, y)] != TileType::Floor) continue;
                if (x == player.x && y == player.y) continue;
                if (IsMerchantAt(x, y)) continue;
                if (x == merchantDoorX && y == merchantDoorY) continue;
                if (IsInsideMerchantChamber(x, y)) continue;

                if (RandomInt(1, 100) <= 3) {
                    if (IsInsideAnyRoom(x, y) && !IsAnyDecorationAt(x, y) && solidVariantCount > 0) {
                        Decoration decoration;
                        decoration.x = x;
                        decoration.y = y;
                        decoration.variant = RandomInt(0, solidVariantCount - 1);
                        solidDecorations.push_back(decoration);
                    }
                    continue;
                }

                if (RandomInt(1, 100) <= 8) {
                    if (!IsAnyDecorationAt(x, y) && walkableVariantCount > 0) {
                        Decoration decoration;
                        decoration.x = x;
                        decoration.y = y;
                        decoration.variant = RandomInt(0, walkableVariantCount - 1);
                        walkableDecorations.push_back(decoration);
                    }
                }
            }
        }
    }

    void SpawnInteractables() {
        interactables.clear();

        int stairsX = -1;
        int stairsY = -1;
        if (!rooms.empty()) {
            auto stairsPos = RoomCenter(rooms.back());
            stairsX = stairsPos.first;
            stairsY = stairsPos.second;
        }

        auto spawnType = [&](InteractableType type, int count) {
            bool requireRoomTile = (type == InteractableType::Chest || type == InteractableType::Pot);
            for (int i = 0; i < count; ++i) {
                bool placed = false;
                for (int attempts = 0; attempts < 160; ++attempts) {
                    int ix = RandomInt(1, kMapWidth - 2);
                    int iy = RandomInt(1, kMapHeight - 2);
                    if (!IsWalkable(ix, iy)) continue;
                    if (ix == player.x && iy == player.y) continue;
                    if (ix == stairsX && iy == stairsY) continue;
                    if (IsMerchantAt(ix, iy)) continue;
                    if (ix == merchantDoorX && iy == merchantDoorY) continue;
                    if (IsInsideMerchantChamber(ix, iy)) continue;
                    if (requireRoomTile && !IsInsideAnyRoom(ix, iy)) continue;
                    if (IsAnyDecorationAt(ix, iy)) continue;
                    if (IsAnyInteractableAt(ix, iy)) continue;

                    Interactable interactable;
                    interactable.x = ix;
                    interactable.y = iy;
                    interactable.type = type;
                    interactable.opened = false;
                    interactables.push_back(interactable);
                    placed = true;
                    break;
                }

                if (!placed) break;
            }
        };

        spawnType(InteractableType::Chest, RandomInt(0, 2));
        spawnType(InteractableType::Pot, RandomInt(0, 10));
        spawnType(InteractableType::Button, RandomInt(0, 2));

        std::vector<std::pair<int, int>> hallwayCandidates;
        std::vector<std::pair<int, int>> roomFloorCandidates;
        for (int y = 1; y < kMapHeight - 1; ++y) {
            for (int x = 1; x < kMapWidth - 1; ++x) {
                if (!IsWalkable(x, y)) continue;
                if (x == player.x && y == player.y) continue;
                if (x == stairsX && y == stairsY) continue;
                if (IsMerchantAt(x, y)) continue;
                if (x == merchantDoorX && y == merchantDoorY) continue;
                if (IsInsideMerchantChamber(x, y)) continue;
                if (IsAnyDecorationAt(x, y)) continue;
                if (IsAnyInteractableAt(x, y)) continue;

                if (IsInsideAnyRoom(x, y)) {
                    roomFloorCandidates.push_back({x, y});
                } else {
                    hallwayCandidates.push_back({x, y});
                }
            }
        }

        int spikeCount = RandomInt(2, 8);
        int spikesInHallways = static_cast<int>(std::round(spikeCount * 0.80f));

        auto spawnFromCandidates = [&](std::vector<std::pair<int, int>>& candidates, int count) {
            int placed = 0;
            while (placed < count && !candidates.empty()) {
                int pick = RandomInt(0, static_cast<int>(candidates.size()) - 1);
                int sx = candidates[pick].first;
                int sy = candidates[pick].second;
                candidates[pick] = candidates.back();
                candidates.pop_back();

                if (IsAnyInteractableAt(sx, sy)) continue;

                Interactable spike;
                spike.x = sx;
                spike.y = sy;
                spike.type = InteractableType::Spike;
                spike.opened = false;
                interactables.push_back(spike);
                placed += 1;
            }
            return placed;
        };

        int placedHallwaySpikes = spawnFromCandidates(hallwayCandidates, spikesInHallways);
        int remainingSpikes = spikeCount - placedHallwaySpikes;
        if (remainingSpikes > 0) {
            int placedRoomSpikes = spawnFromCandidates(roomFloorCandidates, remainingSpikes);
            remainingSpikes -= placedRoomSpikes;
        }
        if (remainingSpikes > 0) {
            spawnFromCandidates(hallwayCandidates, remainingSpikes);
        }
    }

    void GenerateLevel(bool firstLevel) {
        tiles.assign(kMapWidth * kMapHeight, TileType::Outside);
        rooms.clear();

        constexpr int kMaxRoomAttempts = 180;
        constexpr int kMinRooms = 7;
        constexpr int kMaxRooms = 13;
        int targetRooms = RandomInt(kMinRooms, kMaxRooms);

        for (int attempt = 0; attempt < kMaxRoomAttempts && static_cast<int>(rooms.size()) < targetRooms; ++attempt) {
            Room room;
            room.w = RandomInt(4, 8);
            room.h = RandomInt(4, 8);
            room.x = RandomInt(1, kMapWidth - room.w - 2);
            room.y = RandomInt(1, kMapHeight - room.h - 2);

            Room expanded{room.x - 1, room.y - 1, room.w + 2, room.h + 2};

            bool blocked = false;
            for (const Room& placed : rooms) {
                if (RoomsOverlap(expanded, placed)) {
                    blocked = true;
                    break;
                }
            }
            if (blocked) continue;

            CarveRoom(room);
            if (!rooms.empty()) {
                auto [x1, y1] = RoomCenter(rooms.back());
                auto [x2, y2] = RoomCenter(room);
                CarveCorridor(x1, y1, x2, y2);
            }
            rooms.push_back(room);
        }

        if (rooms.empty()) {
            Room fallback{4, 4, 8, 8};
            CarveRoom(fallback);
            rooms.push_back(fallback);
        }

        auto [startX, startY] = RoomCenter(rooms.front());
        player.x = startX;
        player.y = startY;

        auto [stairsX, stairsY] = RoomCenter(rooms.back());
        tiles[Index(stairsX, stairsY)] = TileType::Stairs;

        BuildWallsFromOutside();
        SetupMerchantForFloor();
        SpawnDecorations();
        SpawnInteractables();
        SpawnEnemies();
        SpawnGoldStacks();
        EnsureProgressionPathToStairs(stairsX, stairsY);

        if (!firstLevel) {
            int healAmount = std::max(2, player.maxHp / 6);
            player.hp = std::min(player.maxHp, player.hp + healAmount);
            AddLog("You descend to floor " + std::to_string(dungeonLevel) + ".");
        } else {
            AddLog("Find the stairs to go deeper.");
        }
    }

    int RollDamage(int attack) {
        int low = std::max(1, attack - 1);
        int high = std::max(low, attack + 1);
        return RandomInt(low, high);
    }

    void GainXp(int amount) {
        PassiveRuntimeEffects effects = ComputePassiveEffects();
        int adjustedAmount = std::max(0, static_cast<int>(std::round(static_cast<float>(amount) * std::max(0.05f, effects.expMult))));
        player.xp += adjustedAmount;
        while (player.xp >= player.xpToNext) {
            player.xp -= player.xpToNext;
            player.level += 1;
            player.skillPoints += 1;
            player.xpToNext = static_cast<int>(std::round(player.xpToNext * 1.4f));
            player.maxHp += 4;
            player.hp = std::min(player.maxHp, player.hp + 4);
            player.atk += 1;
            AddLog("Level up! You are now level " + std::to_string(player.level) + ".");
            AddLog("Skill point gained. Press K to open skills.");
            if (player.level % 5 == 0) {
                pendingPassiveChoices += 1;
                AddLog("Passive draft unlocked.");
            }
            RecomputePlayerStats();
        }

        if (!passiveChoiceOpen && pendingPassiveChoices > 0) {
            pendingPassiveChoices -= 1;
            GeneratePassiveChoiceOffer();
        }
    }

    bool PlayerTryMove(int dx, int dy, bool& changedFloor) {
        changedFloor = false;
        int targetX = player.x + dx;
        int targetY = player.y + dy;

        if (!InBounds(targetX, targetY)) return false;

        int enemyIndex = -1;
        if (IsOccupiedByEnemy(targetX, targetY, &enemyIndex)) {
            DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk));
            return true;
        }

        if (IsMerchantAt(targetX, targetY)) {
            PlaySfx(sfxInteract, sfxInteractLoaded);
            merchantOpen = true;
            merchantTab = MerchantTab::Buy;
            merchantSelection = 0;
            return false;
        }

        if (IsMerchantChamberLockedTile(targetX, targetY)) {
            AddLog("The Merchant chamber is sealed. Defeat all enemies first.");
            return false;
        }

        if (!IsWalkable(targetX, targetY)) return false;

        player.x = targetX;
        player.y = targetY;

        TryPressButtonAt(player.x, player.y);
        TryTriggerSpikeAtPlayer(player.x, player.y);

        for (auto it = goldStacks.begin(); it != goldStacks.end(); ) {
            if (it->x == player.x && it->y == player.y) {
                player.gold += it->amount;
                PlaySfx(sfxGold, sfxGoldLoaded, true, 0.97f, 1.06f);
                AddLog("Picked up " + std::to_string(it->amount) + " gold.");
                it = goldStacks.erase(it);
            } else {
                ++it;
            }
        }

        if (tiles[Index(player.x, player.y)] == TileType::Stairs) {
            PlaySfx(sfxStairs, sfxStairsLoaded);
            dungeonLevel += 1;
            score += 25;
            changedFloor = true;
            GenerateLevel(false);
        }

        return true;
    }

    bool PositionBlockedForEnemy(int x, int y, int selfIndex) {
        if (player.x == x && player.y == y) return true;
        for (int i = 0; i < static_cast<int>(enemies.size()); ++i) {
            if (i == selfIndex) continue;
            if (enemies[i].alive && enemies[i].x == x && enemies[i].y == y) return true;
        }
        return false;
    }

    bool IsWarriorArchetype(EnemyArchetype archetype) const {
        return archetype == EnemyArchetype::GoblinWarrior || archetype == EnemyArchetype::UndeadWarrior;
    }

    bool IsRangerArchetype(EnemyArchetype archetype) const {
        return archetype == EnemyArchetype::GoblinRanger || archetype == EnemyArchetype::UndeadRanger;
    }

    bool IsWizardArchetype(EnemyArchetype archetype) const {
        return archetype == EnemyArchetype::GoblinWizard || archetype == EnemyArchetype::UndeadWizard;
    }

    bool IsBeastArchetype(EnemyArchetype archetype) const {
        return archetype == EnemyArchetype::Rat || archetype == EnemyArchetype::Wolf || archetype == EnemyArchetype::Bat;
    }

    const char* EnemyName(EnemyArchetype archetype) const {
        switch (archetype) {
        case EnemyArchetype::GoblinWarrior: return "Goblin Warrior";
        case EnemyArchetype::GoblinRanger: return "Goblin Ranger";
        case EnemyArchetype::GoblinWizard: return "Goblin Wizard";
        case EnemyArchetype::UndeadWarrior: return "Undead Warrior";
        case EnemyArchetype::UndeadRanger: return "Undead Ranger";
        case EnemyArchetype::UndeadWizard: return "Undead Wizard";
        case EnemyArchetype::Rat: return "Rat";
        case EnemyArchetype::Wolf: return "Wolf";
        case EnemyArchetype::Bat: return "Bat";
        default: return "Enemy";
        }
    }

    bool IsAlignedForRanger(const Enemy& e) const {
        int dx = player.x - e.x;
        int dy = player.y - e.y;
        return (dx == 0) || (dy == 0) || (std::abs(dx) == std::abs(dy));
    }

    bool TryMoveEnemyForRangerAlign(Enemy& e, int selfIndex) {
        std::vector<std::pair<int, int>> dirs = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        std::shuffle(dirs.begin(), dirs.end(), rng);

        int bestX = e.x;
        int bestY = e.y;
        int bestScore = std::numeric_limits<int>::min();

        for (const auto& d : dirs) {
            int nx = e.x + d.first;
            int ny = e.y + d.second;
            if (!IsWalkableForEnemy(nx, ny, selfIndex)) continue;
            if (nx == player.x && ny == player.y) continue;

            int dx = player.x - nx;
            int dy = player.y - ny;
            bool aligned = (dx == 0) || (dy == 0) || (std::abs(dx) == std::abs(dy));
            int dist = std::abs(dx) + std::abs(dy);

            int score = (aligned ? 1000 : 0) - dist;
            if (score > bestScore) {
                bestScore = score;
                bestX = nx;
                bestY = ny;
            }
        }

        if (bestX == e.x && bestY == e.y) return false;
        MoveEnemyAndResolveFloorActions(e, bestX, bestY);
        return true;
    }

    void DamagePlayerFromEnemy(const Enemy& e, const std::string& sourceLabel) {
        int rawDamage = RollDamage(e.atk);
        int damageTaken = std::max(1, rawDamage - player.armor);
        player.hp -= damageTaken;
        if (IsBeastArchetype(e.archetype)) {
            PlayRandomSfx({
                {&sfxBeastAttack, &sfxBeastAttackLoaded},
                {&sfxBeastAttack2, &sfxBeastAttack2Loaded}
            }, true, 0.94f, 1.06f);
        }
        PlayRandomSfx({
            {&sfxHurt, &sfxHurtLoaded},
            {&sfxHurt2, &sfxHurt2Loaded},
            {&sfxHurt3, &sfxHurt3Loaded}
        }, true, 0.94f, 1.05f);

        if (player.armor > 0 && rawDamage > damageTaken) {
            int blocked = rawDamage - damageTaken;
            AddLog(sourceLabel + " hits you for " + std::to_string(damageTaken) + " (" + std::to_string(blocked) + " blocked).");
        } else {
            AddLog(sourceLabel + " hits you for " + std::to_string(damageTaken) + ".");
        }

        if (e.archetype == EnemyArchetype::Rat) {
            if (!player.poisoned) {
                player.poisoned = true;
                TriggerPoisonAfflictFlash();
                AddLog("You are Poisoned!");
            }
        } else if (e.archetype == EnemyArchetype::Wolf) {
            if (!player.bleeding) {
                player.bleeding = true;
                TriggerBleedAfflictFlash();
                AddLog("You are Bleeding!");
            }
        }
    }

    void StartRangerTelegraph(Enemy& e) {
        int dx = player.x - e.x;
        int dy = player.y - e.y;
        e.telegraphDx = (dx > 0) ? 1 : (dx < 0 ? -1 : 0);
        e.telegraphDy = (dy > 0) ? 1 : (dy < 0 ? -1 : 0);
        e.telegraphTargetX = 0;
        e.telegraphTargetY = 0;
        e.telegraph = EnemyTelegraph::RangerLine;
    }

    bool StartWizardTelegraph(Enemy& e) {
        std::vector<std::pair<int, int>> candidates;
        const int offsets[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};

        for (const auto& offset : offsets) {
            int targetX = player.x + offset[0];
            int targetY = player.y + offset[1];
            if (!InBounds(targetX, targetY)) continue;
            if (IsTileOpaque(targetX, targetY)) continue;
            if (!HasLineOfSight(e.x, e.y, targetX, targetY)) continue;
            candidates.push_back({targetX, targetY});
        }

        if (candidates.empty()) return false;

        const auto& picked = candidates[RandomInt(0, static_cast<int>(candidates.size()) - 1)];
        e.telegraphTargetX = picked.first;
        e.telegraphTargetY = picked.second;
        e.telegraphDx = 0;
        e.telegraphDy = 0;
        e.telegraph = EnemyTelegraph::WizardPlus;
        return true;
    }

    bool PlayerInRangerLine(const Enemy& e) const {
        if (e.telegraphDx == 0 && e.telegraphDy == 0) return false;

        int x = e.x + e.telegraphDx;
        int y = e.y + e.telegraphDy;
        while (InBounds(x, y) && !IsTileOpaque(x, y)) {
            if (player.x == x && player.y == y) {
                return true;
            }
            x += e.telegraphDx;
            y += e.telegraphDy;
        }
        return false;
    }

    bool PlayerInWizardPlus(const Enemy& e) const {
        if (player.x == e.telegraphTargetX + 1 && player.y == e.telegraphTargetY) return true;
        if (player.x == e.telegraphTargetX - 1 && player.y == e.telegraphTargetY) return true;
        if (player.x == e.telegraphTargetX && player.y == e.telegraphTargetY + 1) return true;
        if (player.x == e.telegraphTargetX && player.y == e.telegraphTargetY - 1) return true;
        return false;
    }

    void MoveEnemyAndResolveFloorActions(Enemy& enemy, int nx, int ny) {
        enemy.x = nx;
        enemy.y = ny;
        TryTriggerSpikeAtEnemy(enemy);
    }

    void ResolveEnemyTelegraph(Enemy& e) {
        const std::string name = EnemyName(e.archetype);
        if (e.telegraph == EnemyTelegraph::RangerLine) {
            if (PlayerInRangerLine(e)) {
                DamagePlayerFromEnemy(e, name);
            } else {
                AddLog(name + "'s shot misses.");
            }
        } else if (e.telegraph == EnemyTelegraph::WizardPlus) {
            if (PlayerInWizardPlus(e)) {
                DamagePlayerFromEnemy(e, name);
            } else {
                AddLog(name + "'s spell misses.");
            }
        }

        e.telegraph = EnemyTelegraph::None;
    }

    void EnemyTurn() {
        for (int i = 0; i < static_cast<int>(enemies.size()); ++i) {
            Enemy& e = enemies[i];
            if (!e.alive) continue;

            e.isFleeingNow = false;

            if (e.telegraph != EnemyTelegraph::None) {
                ResolveEnemyTelegraph(e);
                if (IsWizardArchetype(e.archetype)) {
                    e.staffCharge = std::min(5, e.staffCharge + 1);
                }
                continue;
            }

            const int dist = std::abs(e.x - player.x) + std::abs(e.y - player.y);
            const bool canSeePlayer = (dist <= kEnemyViewRange) && HasLineOfSight(e.x, e.y, player.x, player.y);
            e.seesPlayer = canSeePlayer;

            if (IsWarriorArchetype(e.archetype) || IsBeastArchetype(e.archetype)) {
                if (dist == 1) {
                    DamagePlayerFromEnemy(e, EnemyName(e.archetype));
                    continue;
                }

                if (canSeePlayer) {
                    auto path = FindPathAStar(e.x, e.y, player.x, player.y, i);
                    if (path.size() >= 2) {
                        int nx = path[1].first;
                        int ny = path[1].second;
                        if (!(nx == player.x && ny == player.y) && !IsEnemyAt(nx, ny, i)) {
                            MoveEnemyAndResolveFloorActions(e, nx, ny);
                        }
                    }
                } else {
                    RandomRoamEnemy(e, i);
                }
                continue;
            }

            if (IsRangerArchetype(e.archetype)) {
                if (canSeePlayer && IsAlignedForRanger(e)) {
                    StartRangerTelegraph(e);
                    AddLog(std::string(EnemyName(e.archetype)) + " draws a bead!");
                    continue;
                }

                if (canSeePlayer) {
                    if (!TryMoveEnemyForRangerAlign(e, i)) {
                        auto path = FindPathAStar(e.x, e.y, player.x, player.y, i);
                        if (path.size() >= 2) {
                            int nx = path[1].first;
                            int ny = path[1].second;
                            if (!(nx == player.x && ny == player.y) && !IsEnemyAt(nx, ny, i)) {
                                MoveEnemyAndResolveFloorActions(e, nx, ny);
                            }
                        }
                    }
                } else {
                    RandomRoamEnemy(e, i);
                }
                continue;
            }

            if (IsWizardArchetype(e.archetype)) {
                if (canSeePlayer) {
                    if (e.staffCharge >= 5) {
                        if (StartWizardTelegraph(e)) {
                            e.staffCharge = 0;
                            AddLog(std::string(EnemyName(e.archetype)) + " begins a spell!");
                        } else {
                            RandomRoamEnemy(e, i);
                            e.staffCharge = std::min(5, e.staffCharge + 1);
                        }
                    } else {
                        RandomRoamEnemy(e, i);
                        e.staffCharge = std::min(5, e.staffCharge + 1);
                    }
                } else {
                    RandomRoamEnemy(e, i);
                    e.staffCharge = std::min(5, e.staffCharge + 1);
                }
                continue;
            }
        }
    }

    bool RollChance(float p) {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(rng) < p;
    }

    bool ShouldFlee(const Enemy& e) {
        if (!e.alive) return false;
        if (e.panicFlee) return true;                  // ALWAYS flee
        if (e.maxHp <= 0) return false;
        if (e.hp * 2 > e.maxHp) return false;          // not below 50%
        return RollChance(0.40f);                      // sometimes flee
    }

    bool TryMoveEnemyAway(Enemy& e, int selfIndex) {
        const std::array<std::array<int, 2>, 4> dirs = {{{1,0},{-1,0},{0,1},{0,-1}}};
        int bestX = e.x, bestY = e.y;
        int bestDist = std::abs(e.x - player.x) + std::abs(e.y - player.y);

        for (const auto& d : dirs) {
            int nx = e.x + d[0];
            int ny = e.y + d[1];
            if (!IsWalkableForEnemy(nx, ny, selfIndex)) continue;
            if (nx == player.x && ny == player.y) continue;

            int nd = std::abs(nx - player.x) + std::abs(ny - player.y);
            if (nd > bestDist) {
                bestDist = nd;
                bestX = nx;
                bestY = ny;
            }
        }

        if (bestX == e.x && bestY == e.y) return false;
        MoveEnemyAndResolveFloorActions(e, bestX, bestY);
        return true;
    }

    void RandomRoamEnemy(Enemy& e, int selfIndex) {
        std::uniform_int_distribution<int> stayRoll(0, 1);
        if (stayRoll(rng) == 0) return;

        std::vector<std::pair<int, int>> dirs = {
            { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }
        };
        std::shuffle(dirs.begin(), dirs.end(), rng);

        for (const auto& d : dirs) {
            int nx = e.x + d.first;
            int ny = e.y + d.second;
            if (!IsWalkableForEnemy(nx, ny, selfIndex)) continue;
            if (nx == player.x && ny == player.y) continue;
            MoveEnemyAndResolveFloorActions(e, nx, ny);
            return;
        }
    }

    void DrawMap() const {
        for (int y = 0; y < kMapHeight; ++y) {
            for (int x = 0; x < kMapWidth; ++x) {
                Rectangle rect{static_cast<float>(x * kTileSize), static_cast<float>(y * kTileSize), static_cast<float>(kTileSize), static_cast<float>(kTileSize)};
                TileType tile = tiles[Index(x, y)];
                Rectangle roomTileSrc{};
                bool drewAtlasTile = false;
                if (TryGetRoomTileSource(tile, x, y, roomTileSrc)) {
                    DrawTexturePro(roomAtlas, roomTileSrc, rect, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                    drewAtlasTile = true;
                }

                if (!drewAtlasTile) {
                    if (tile == TileType::Outside) DrawRectangleRec(rect, Color{10, 10, 14, 255});
                    if (tile == TileType::Wall) DrawRectangleRec(rect, Color{18, 18, 25, 255});
                    if (tile == TileType::Floor) DrawRectangleRec(rect, Color{38, 38, 52, 255});
                }

                if (tile == TileType::Floor || tile == TileType::Stairs) {
                    const Decoration* solidDecoration = nullptr;
                    const Decoration* walkableDecoration = nullptr;
                    for (const Decoration& decoration : walkableDecorations) {
                        if (decoration.x == x && decoration.y == y) {
                            walkableDecoration = &decoration;
                            break;
                        }
                    }
                    for (const Decoration& decoration : solidDecorations) {
                        if (decoration.x == x && decoration.y == y) {
                            solidDecoration = &decoration;
                            break;
                        }
                    }

                    if (walkableDecoration != nullptr) {
                        Rectangle walkableSrc{};
                        if (TryGetWalkableDecorationSource(walkableDecoration->variant, walkableSrc)) {
                            DrawTexturePro(walkablesAtlas, walkableSrc, rect, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                        } else {
                            DrawCircle(x * kTileSize + kTileSize / 2, y * kTileSize + kTileSize / 2, 3.0f, Color{110, 160, 90, 220});
                        }
                    }

                    if (solidDecoration != nullptr) {
                        Rectangle solidSrc{};
                        if (TryGetSolidDecorationSource(solidDecoration->variant, solidSrc)) {
                            DrawTexturePro(solidsAtlas, solidSrc, rect, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                        } else {
                            DrawRectangle(x * kTileSize + 7, y * kTileSize + 7, kTileSize - 14, kTileSize - 14, Color{90, 86, 78, 235});
                        }
                    }
                }

                if (tile == TileType::Stairs) {
                    if (!drewAtlasTile) {
                        DrawRectangleRec(rect, Color{60, 76, 48, 255});
                    } else {
                        DrawRectangleRec(rect, Fade(Color{60, 76, 48, 255}, 0.28f));
                    }
                    if (stairsIconLoaded) {
                        Rectangle stairsSrc{0.0f, 0.0f, static_cast<float>(stairsIcon.width), static_cast<float>(stairsIcon.height)};
                        Rectangle stairsDst{static_cast<float>(x * kTileSize), static_cast<float>(y * kTileSize), static_cast<float>(kTileSize), static_cast<float>(kTileSize)};
                        DrawTexturePro(stairsIcon, stairsSrc, stairsDst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                    } else {
                        const int stairsFontSize = 20;
                        const char* stairsText = ">";
                        const int textW = MeasureText(stairsText, stairsFontSize);
                        const int textX = x * kTileSize + (kTileSize - textW) / 2;
                        const int textY = y * kTileSize + (kTileSize - stairsFontSize) / 2;
                        DrawText(stairsText, textX, textY, stairsFontSize, Color{210, 255, 140, 255});
                    }
                }

                const Interactable* interactable = nullptr;
                for (const Interactable& candidate : interactables) {
                    if (candidate.x == x && candidate.y == y) {
                        interactable = &candidate;
                        break;
                    }
                }

                if (interactable != nullptr) {
                    Rectangle actionSrc{};
                    int actionStateCol = interactable->opened ? 1 : 0;
                    int actionStateRow = 2;
                    if (interactable->type == InteractableType::Chest) actionStateRow = 2;
                    if (interactable->type == InteractableType::Pot) actionStateRow = 3;
                    if (interactable->type == InteractableType::Button) actionStateRow = 4;
                    if (interactable->type == InteractableType::Spike) actionStateRow = 5;
                    if (TryGetActionIconSource(actionStateCol, actionStateRow, actionSrc)) {
                        DrawTexturePro(actionAtlas, actionSrc, rect, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                    } else if (interactable->type == InteractableType::Chest) {
                        Color fill = interactable->opened ? Color{114, 88, 52, 220} : Color{145, 108, 56, 235};
                        DrawRectangle(x * kTileSize + 5, y * kTileSize + 7, kTileSize - 10, kTileSize - 12, fill);
                        DrawRectangleLines(x * kTileSize + 5, y * kTileSize + 7, kTileSize - 10, kTileSize - 12, Color{58, 36, 18, 240});
                    } else if (interactable->type == InteractableType::Pot) {
                        Color fill = interactable->opened ? Color{94, 94, 104, 220} : Color{126, 126, 138, 235};
                        DrawCircle(x * kTileSize + kTileSize / 2, y * kTileSize + kTileSize / 2, 8.0f, fill);
                        DrawCircleLines(x * kTileSize + kTileSize / 2, y * kTileSize + kTileSize / 2, 8.0f, Color{62, 62, 72, 240});
                    } else if (interactable->type == InteractableType::Button) {
                        Color fill = interactable->opened ? Color{90, 170, 90, 235} : Color{160, 170, 180, 235};
                        DrawRectangle(x * kTileSize + 9, y * kTileSize + 9, kTileSize - 18, kTileSize - 18, fill);
                        DrawRectangleLines(x * kTileSize + 9, y * kTileSize + 9, kTileSize - 18, kTileSize - 18, Color{42, 52, 46, 240});
                    } else {
                        if (interactable->opened) {
                            DrawRectangle(x * kTileSize + 6, y * kTileSize + 16, kTileSize - 12, 6, Color{120, 120, 130, 230});
                        } else {
                            DrawTriangle(
                                Vector2{static_cast<float>(x * kTileSize + 8), static_cast<float>(y * kTileSize + kTileSize - 6)},
                                Vector2{static_cast<float>(x * kTileSize + kTileSize / 2), static_cast<float>(y * kTileSize + 6)},
                                Vector2{static_cast<float>(x * kTileSize + kTileSize - 8), static_cast<float>(y * kTileSize + kTileSize - 6)},
                                Color{185, 185, 200, 240}
                            );
                        }
                    }
                }
                DrawRectangleLinesEx(rect, 1.0f, Color{10, 10, 14, 255});
            }
        }

        if (merchantPresent && merchantDoorX >= 0 && merchantDoorY >= 0) {
            int dx = merchantDoorX * kTileSize;
            int dy = merchantDoorY * kTileSize;

            Rectangle actionSrc{};
            int actionStateCol = merchantRoomOpened ? 1 : 0;
            int actionStateRow = UseWoodDoorTheme() ? 0 : 1;
            if (TryGetActionIconSource(actionStateCol, actionStateRow, actionSrc)) {
                Rectangle actionDst{static_cast<float>(dx), static_cast<float>(dy), static_cast<float>(kTileSize), static_cast<float>(kTileSize)};
                DrawTexturePro(actionAtlas, actionSrc, actionDst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
            } else {
                if (!merchantRoomOpened) {
                    DrawRectangle(dx + 5, dy + 5, kTileSize - 10, kTileSize - 10, Color{122, 86, 32, 235});
                    DrawRectangleLines(dx + 5, dy + 5, kTileSize - 10, kTileSize - 10, Color{235, 188, 96, 255});

                    int lockW = MeasureText("X", 20);
                    DrawText("X", dx + (kTileSize - lockW) / 2, dy + 4, 20, Color{35, 20, 8, 255});
                } else {
                    DrawRectangleLines(dx + 4, dy + 4, kTileSize - 8, kTileSize - 8, Color{245, 220, 150, 230});
                    int archW = MeasureText("^", 18);
                    DrawText("^", dx + (kTileSize - archW) / 2, dy + 6, 18, Color{245, 220, 150, 230});
                }
            }
        }
    }

    void DrawMapStatusOutline() const {
        const int mapW = kMapWidth * kTileSize;
        const int mapH = kMapHeight * kTileSize;

        if (player.poisoned) {
            DrawRectangleLinesEx(Rectangle{1.0f, 1.0f, static_cast<float>(mapW - 2), static_cast<float>(mapH - 2)}, 3.0f, Color{30, 220, 80, 230});
        }

        if (player.bleeding) {
            float offset = player.poisoned ? 5.0f : 1.0f;
            float sizeOffset = player.poisoned ? 10.0f : 2.0f;
            DrawRectangleLinesEx(Rectangle{offset, offset, static_cast<float>(mapW) - sizeOffset, static_cast<float>(mapH) - sizeOffset}, 3.0f, Color{220, 40, 40, 230});
        }
    }

    void DrawGoldStacks() const {
        for (const GoldStack& stack : goldStacks) {
            int x = stack.x * kTileSize;
            int y = stack.y * kTileSize;

            int iconCol = 0;
            if (stack.type == GoldStackType::Small) iconCol = 0;
            if (stack.type == GoldStackType::Medium) iconCol = 1;
            if (stack.type == GoldStackType::Large) iconCol = 2;

            Rectangle goldSrc{};
            if (TryGetGoldIconSource(iconCol, goldSrc)) {
                Rectangle goldDst{static_cast<float>(x + 4), static_cast<float>(y + 4), static_cast<float>(kTileSize - 8), static_cast<float>(kTileSize - 8)};
                DrawTexturePro(iconAtlas, goldSrc, goldDst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
            } else {
                Color fallback = GOLD;
                if (stack.type == GoldStackType::Small) fallback = Color{220, 180, 40, 255};
                if (stack.type == GoldStackType::Medium) fallback = Color{230, 190, 60, 255};
                if (stack.type == GoldStackType::Large) fallback = Color{255, 215, 90, 255};
                DrawCircle(x + kTileSize / 2, y + kTileSize / 2, 7.0f, fallback);
            }
        }
    }

    void DrawAttackPreview() const {
        if (inventoryOpen || gameOver || classSelectionPending) return;

        std::vector<std::pair<int, int>> preview = GetAttackPreviewTiles();
        WeaponType weaponType = CurrentWeaponType();

        for (const auto& tile : preview) {
            int x = tile.first;
            int y = tile.second;
            if (!InBounds(x, y)) continue;

            Color color = Color{255, 120, 120, 120};
            if (weaponType == WeaponType::Bow) color = Color{120, 180, 255, 120};
            if (weaponType == WeaponType::Staff) color = player.staffCharge >= 5 ? Color{180, 120, 255, 130} : Color{100, 100, 100, 110};

            DrawRectangle(x * kTileSize + 2, y * kTileSize + 2, kTileSize - 4, kTileSize - 4, color);
            DrawRectangleLines(x * kTileSize + 2, y * kTileSize + 2, kTileSize - 4, kTileSize - 4, Fade(color, 0.9f));
        }
    }

    void DrawEnemyTelegraphs() const {
        if (inventoryOpen || gameOver || classSelectionPending) return;

        for (const Enemy& e : enemies) {
            if (!e.alive || e.telegraph == EnemyTelegraph::None) continue;

            if (e.telegraph == EnemyTelegraph::RangerLine) {
                if (e.telegraphDx == 0 && e.telegraphDy == 0) continue;
                int x = e.x + e.telegraphDx;
                int y = e.y + e.telegraphDy;
                while (InBounds(x, y) && !IsTileOpaque(x, y)) {
                    DrawRectangle(x * kTileSize + 2, y * kTileSize + 2, kTileSize - 4, kTileSize - 4, Color{255, 50, 50, 105});
                    DrawRectangleLines(x * kTileSize + 2, y * kTileSize + 2, kTileSize - 4, kTileSize - 4, Color{255, 80, 80, 185});
                    x += e.telegraphDx;
                    y += e.telegraphDy;
                }
            } else if (e.telegraph == EnemyTelegraph::WizardPlus) {
                std::array<std::pair<int, int>, 5> plusTiles = {{
                    {e.telegraphTargetX, e.telegraphTargetY},
                    {e.telegraphTargetX + 1, e.telegraphTargetY},
                    {e.telegraphTargetX - 1, e.telegraphTargetY},
                    {e.telegraphTargetX, e.telegraphTargetY + 1},
                    {e.telegraphTargetX, e.telegraphTargetY - 1}
                }};
                for (const auto& tile : plusTiles) {
                    if (!InBounds(tile.first, tile.second)) continue;
                    DrawRectangle(tile.first * kTileSize + 2, tile.second * kTileSize + 2, kTileSize - 4, kTileSize - 4, Color{255, 50, 50, 115});
                    DrawRectangleLines(tile.first * kTileSize + 2, tile.second * kTileSize + 2, kTileSize - 4, kTileSize - 4, Color{255, 80, 80, 200});
                }
            }
        }
    }

    void DrawEntities() const {
        // Draw enemies
        for (const Enemy& e : enemies) {
            if (!e.alive) continue;

            int ex = e.x * kTileSize;
            int ey = e.y * kTileSize;

            Rectangle enemyIconSrc{};
            if (TryGetEnemyIconSource(e.iconVariant, enemyIconSrc)) {
                Rectangle enemyDst{static_cast<float>(ex + 3), static_cast<float>(ey + 3), static_cast<float>(kTileSize - 6), static_cast<float>(kTileSize - 6)};
                DrawTexturePro(enemyAtlas, enemyIconSrc, enemyDst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
            } else {
                DrawRectangle(ex + 6, ey + 6, kTileSize - 12, kTileSize - 12, MAROON);
            }

            // Status marker above enemy
            const int cx = ex + kTileSize / 2;
            const int yAbove = std::max(0, ey - 20);

            if (e.isFleeingNow) {
                if ((static_cast<int>(GetTime() * 8.0) % 2) == 0) {
                    const char* txt = "!!!";
                    int fs = 20;
                    int tw = MeasureText(txt, fs);
                    DrawText(txt, cx - tw / 2, yAbove, fs, YELLOW);
                }
            } else if (e.seesPlayer) {
                const char* txt = "!";
                int fs = 24;
                int tw = MeasureText(txt, fs);
                DrawText(txt, cx - tw / 2, yAbove, fs, RED);
            }
        }

        if (merchantPresent) {
            int mx = merchantX * kTileSize;
            int my = merchantY * kTileSize;
            Rectangle npcSrc{};
            if (TryGetNpcIconSource(merchantIconVariant, npcSrc)) {
                Rectangle npcDst{static_cast<float>(mx + 3), static_cast<float>(my + 3), static_cast<float>(kTileSize - 6), static_cast<float>(kTileSize - 6)};
                DrawTexturePro(npcAtlas, npcSrc, npcDst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
            } else {
                DrawRectangle(mx + 4, my + 4, kTileSize - 8, kTileSize - 8, Color{210, 160, 62, 255});
                DrawText("$", mx + 8, my + 3, 24, Color{36, 28, 18, 255});
            }
        }

        // Draw player last (on top)
        int px = player.x * kTileSize;
        int py = player.y * kTileSize;
        Rectangle playerIconSrc{};
        if (TryGetClassIconSource(player.playerClass, playerIconSrc)) {
            Rectangle playerDst{static_cast<float>(px + 3), static_cast<float>(py + 3), static_cast<float>(kTileSize - 6), static_cast<float>(kTileSize - 6)};
            DrawTexturePro(playerAtlas, playerIconSrc, playerDst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        } else {
            Color fallback = SKYBLUE;
            if (player.playerClass == PlayerClass::Warrior) fallback = Color{200, 90, 90, 255};
            if (player.playerClass == PlayerClass::Ranger) fallback = Color{80, 180, 110, 255};
            if (player.playerClass == PlayerClass::Wizard) fallback = Color{120, 150, 230, 255};
            DrawRectangle(px + 5, py + 5, kTileSize - 10, kTileSize - 10, fallback);
        }
    }

    void DrawSkillTreeOverlay() const {
        DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.76f));

        const int panelW = 1020;
        const int panelH = 620;
        const int panelX = (kScreenWidth - panelW) / 2;
        const int panelY = (kScreenHeight - panelH) / 2;

        DrawRectangle(panelX, panelY, panelW, panelH, Color{16, 16, 24, 248});
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{90, 100, 125, 255});

        const char* classLabel = "Warrior";
        if (player.playerClass == PlayerClass::Ranger) classLabel = "Ranger";
        if (player.playerClass == PlayerClass::Wizard) classLabel = "Wizard";

        DrawText(TextFormat("%s Skill Tree", classLabel), panelX + 24, panelY + 14, 36, RAYWHITE);
        DrawText(TextFormat("Skill Points: %i", player.skillPoints), panelX + panelW - 260, panelY + 20, 28, Color{255, 220, 120, 255});
        DrawText("Tree nodes unlock active skills only (no passive stats).", panelX + 24, panelY + 56, 21, LIGHTGRAY);

        const char* branchTitles[3] = {"Offense", "Balanced", "Defense"};
        Color branchColors[3] = {
            Color{210, 90, 90, 255},
            Color{170, 145, 220, 255},
            Color{95, 155, 235, 255}
        };

        const int rootX = panelX + panelW / 2;
        const int rootY = panelY + 110;
        DrawCircle(rootX, rootY, 16.0f, Color{220, 210, 120, 255});
        DrawCircleLines(rootX, rootY, 16.0f, Color{250, 240, 180, 255});
        DrawText("Core", rootX - 24, rootY - 36, 20, LIGHTGRAY);

        const int branchTopY = panelY + 214;
        const int branchBottomY = panelY + 510;
        const int nodeGapY = (branchBottomY - branchTopY) / std::max(1, (kClassSkillMaxRank - 1));
        const int branchSpacing = 300;
        const int leftBranchX = panelX + (panelW - branchSpacing * 2) / 2;

        for (int i = 0; i < kClassSkillCount; ++i) {
            int bx = leftBranchX + i * branchSpacing;
            int rank = player.classSkillRanks[i];
            bool selected = (i == skillSelection);

            DrawLineEx(
                Vector2{static_cast<float>(rootX), static_cast<float>(rootY + 12)},
                Vector2{static_cast<float>(bx), static_cast<float>(branchTopY - 18)},
                4.0f,
                Fade(branchColors[i], 0.75f)
            );

            DrawText(branchTitles[i], bx - 62, panelY + 152, 30, selected ? RAYWHITE : Fade(branchColors[i], 0.95f));
            DrawText(TextFormat("Rank %i/%i", rank, kClassSkillMaxRank), bx - 52, panelY + 184, 20, Color{210, 220, 240, 255});

            for (int p = 0; p < kClassSkillMaxRank; ++p) {
                int nodeY = branchTopY + p * nodeGapY;
                if (p > 0) {
                    DrawLineEx(
                        Vector2{static_cast<float>(bx), static_cast<float>(nodeY - nodeGapY + 14)},
                        Vector2{static_cast<float>(bx), static_cast<float>(nodeY - 14)},
                        3.0f,
                        Color{78, 86, 108, 255}
                    );
                }

                bool filled = p < rank;
                Color nodeFill = filled ? branchColors[i] : Color{48, 54, 68, 255};
                float radius = (selected && p == rank && rank < kClassSkillMaxRank) ? 13.0f : 11.0f;
                DrawCircle(bx, nodeY, radius, nodeFill);
                DrawCircleLines(bx, nodeY, radius, selected ? Color{220, 230, 255, 255} : Color{100, 110, 130, 255});
            }
        }

        const char* selectedName = SkillNameForClass(player.playerClass, skillSelection);
        std::string selectedDesc = SkillDescriptionForClass(player.playerClass, skillSelection);
        Rectangle descPanel{static_cast<float>(panelX + 26), static_cast<float>(panelY + panelH - 110), static_cast<float>(panelW - 52), 74.0f};
        DrawRectangleRec(descPanel, Color{24, 28, 40, 230});
        DrawRectangleLinesEx(descPanel, 1.0f, Color{82, 92, 120, 255});
        DrawText(selectedName, panelX + 40, panelY + panelH - 102, 26, RAYWHITE);
        DrawText(selectedDesc.c_str(), panelX + 40, panelY + panelH - 72, 20, LIGHTGRAY);

        DrawText("A/D or W/S: Select branch   Enter/E/Space: Spend point   K/Esc: Close", panelX + 24, panelY + panelH - 22, 18, LIGHTGRAY);
    }

    void DrawPassiveChoiceOverlay() const {
        if (!passiveChoiceOpen) return;

        DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.82f));

        const int panelW = 960;
        const int panelH = 420;
        const int panelX = (kScreenWidth - panelW) / 2;
        const int panelY = (kScreenHeight - panelH) / 2;

        DrawRectangle(panelX, panelY, panelW, panelH, Color{16, 16, 24, 248});
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{95, 105, 132, 255});

        DrawText("Choose a Passive Skill", panelX + 24, panelY + 18, 38, RAYWHITE);
        DrawText("Unlocked every 5 levels", panelX + panelW - 330, panelY + 30, 24, Color{255, 220, 120, 255});

        const int cardW = 280;
        const int cardH = 220;
        const int gap = 26;
        const int totalCardsW = cardW * 3 + gap * 2;
        const int startX = panelX + (panelW - totalCardsW) / 2;
        const int cardY = panelY + 124;

        for (int i = 0; i < 3; ++i) {
            int x = startX + i * (cardW + gap);
            Rectangle cardRect{static_cast<float>(x), static_cast<float>(cardY), static_cast<float>(cardW), static_cast<float>(cardH)};
            bool selected = (i == passiveChoiceSelection);

            DrawRectangleRec(cardRect, selected ? Color{56, 70, 106, 220} : Color{28, 32, 44, 215});

            const GeneratedPassive& passive = passiveChoiceOptions[static_cast<size_t>(i)];
            Color rarityColor = ItemTierColor(passive.grade);
            DrawRectangleLinesEx(cardRect, 2.0f, rarityColor);
            if (selected) {
                Rectangle innerRect{cardRect.x + 3.0f, cardRect.y + 3.0f, cardRect.width - 6.0f, cardRect.height - 6.0f};
                DrawRectangleLinesEx(innerRect, 2.0f, Color{195, 220, 255, 255});
            }

            std::string perkName = PassiveSkillName(passive);
            std::string perkDesc = PassiveSkillDescription(passive);

            DrawText(TextFormat("[%i]", i + 1), x + 12, cardY + 10, 26, LIGHTGRAY);
            DrawText(PassiveGradeName(passive.grade), x + cardW - 108, cardY + 14, 20, rarityColor);
            int titleSize = 24;
            while (titleSize > 16 && MeasureText(perkName.c_str(), titleSize) > (cardW - 24)) {
                titleSize -= 1;
            }
            DrawText(perkName.c_str(), x + 14, cardY + 56, titleSize, RAYWHITE);

            int descSize = 20;
            while (descSize > 14 && MeasureText(perkDesc.c_str(), descSize) > (cardW - 24)) {
                descSize -= 1;
            }
            DrawText(perkDesc.c_str(), x + 14, cardY + 112, descSize, Color{185, 205, 240, 255});
        }

        DrawText("A/D or W/S: Select   1/2/3 or Enter/E/Space: Confirm", panelX + 24, panelY + panelH - 28, 22, LIGHTGRAY);
    }

    void DrawInventoryOverlay() const {
        DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.72f));
        Vector2 mousePos = GetMousePosition();

        const int panelX = 90;
        const int panelY = 56;
        const int panelW = kScreenWidth - 180;
        const int panelH = kScreenHeight - 112;

        DrawRectangle(panelX, panelY, panelW, panelH, Color{16, 16, 24, 245});
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{90, 90, 120, 255});

        DrawText("Inventory", panelX + 20, panelY + 14, 34, RAYWHITE);
        DrawText("Arrows/W-S: Select   E/Enter: Equip or Use   I/Esc: Close", panelX + 20, panelY + 54, 20, LIGHTGRAY);
        DrawText("Mouse over items/equipment to inspect", panelX + 20, panelY + 78, 18, GRAY);

        const Item* hoveredEquipItem = nullptr;
        int hoveredInvIndex = -1;
        bool hasEquipComparison = false;
        EquipSlot comparisonSlot = EquipSlot::Head;
        int comparisonDirection = 0; // 1 = up, -1 = down

        const int equipX = panelX + 20;
        const int equipY = panelY + 92;
        const int equipW = panelW - 40;
        const int equipH = 182;

        const int listX = panelX + 20;
        const int listY = equipY + equipH + 14;
        const int listW = panelW - 40;
        const int listH = panelY + panelH - listY - 16;
        const int gridTop = listY + 42;

        int visibleRows = 0;
        int totalRows = 0;
        int maxScrollRows = 0;
        int scrollRow = 0;

        if (!player.inventory.empty()) {
            visibleRows = std::max(1, (listH - 52) / (kInventoryCellSize + kInventoryCellGap));
            totalRows = (static_cast<int>(player.inventory.size()) + kInventoryGridCols - 1) / kInventoryGridCols;
            maxScrollRows = std::max(0, totalRows - visibleRows);
            scrollRow = std::clamp(inventoryScrollOffset, 0, maxScrollRows);

            for (int row = 0; row < visibleRows; ++row) {
                for (int col = 0; col < kInventoryGridCols; ++col) {
                    int index = (scrollRow + row) * kInventoryGridCols + col;
                    if (index >= static_cast<int>(player.inventory.size())) continue;

                    int cellX = listX + 10 + col * (kInventoryCellSize + kInventoryCellGap);
                    int cellY = gridTop + row * (kInventoryCellSize + kInventoryCellGap);
                    Rectangle cellRect{static_cast<float>(cellX), static_cast<float>(cellY), static_cast<float>(kInventoryCellSize), static_cast<float>(kInventoryCellSize)};
                    if (CheckCollisionPointRec(mousePos, cellRect)) {
                        hoveredInvIndex = index;
                    }
                }
            }

            if (hoveredInvIndex >= 0) {
                const Item& hoveredItem = player.inventory[hoveredInvIndex];
                if (hoveredItem.category == ItemCategory::Equipment) {
                    EquipSlot targetSlot = hoveredItem.slot;
                    if (hoveredItem.slot == EquipSlot::Ring1 || hoveredItem.slot == EquipSlot::Ring2) {
                        size_t ring1Index = SlotIndex(EquipSlot::Ring1);
                        size_t ring2Index = SlotIndex(EquipSlot::Ring2);
                        if (!player.equipped[ring1Index].has_value()) {
                            targetSlot = EquipSlot::Ring1;
                        } else if (!player.equipped[ring2Index].has_value()) {
                            targetSlot = EquipSlot::Ring2;
                        } else {
                            targetSlot = EquipSlot::Ring1;
                        }
                    }

                    const std::optional<Item>& equippedTarget = player.equipped[SlotIndex(targetSlot)];
                    if (equippedTarget.has_value()) {
                        float diff = hoveredItem.itemPower - equippedTarget->itemPower;
                        if (diff > 0.01f) {
                            hasEquipComparison = true;
                            comparisonSlot = targetSlot;
                            comparisonDirection = 1;
                        } else if (diff < -0.01f) {
                            hasEquipComparison = true;
                            comparisonSlot = targetSlot;
                            comparisonDirection = -1;
                        }
                    }
                }
            }
        }

        DrawRectangle(equipX, equipY, equipW, equipH, Color{24, 24, 34, 220});
        DrawRectangleLines(equipX, equipY, equipW, equipH, Color{70, 70, 94, 255});
        DrawText("Equipped", equipX + 10, equipY + 8, 24, RAYWHITE);

        const EquipSlot displaySlots[9] = {
            EquipSlot::Head, EquipSlot::Top, EquipSlot::Feet, EquipSlot::Hands,
            EquipSlot::Ring1, EquipSlot::Ring2, EquipSlot::Necklace, EquipSlot::Weapon, EquipSlot::Shield
        };

        int slotLine = 0;
        for (EquipSlot slot : displaySlots) {
            int column = slotLine % 2;
            int row = slotLine / 2;
            int lineX = equipX + 14 + column * (equipW / 2);
            int lineY = equipY + 40 + row * 26;
            Rectangle slotRect{static_cast<float>(lineX - 6), static_cast<float>(lineY - 2), static_cast<float>(equipW / 2 - 24), 24.0f};

            const std::optional<Item>& equippedItem = player.equipped[SlotIndex(slot)];
            bool hovered = CheckCollisionPointRec(mousePos, slotRect);
            if (hasEquipComparison && slot == comparisonSlot) {
                Color compareColor = comparisonDirection > 0 ? Color{42, 92, 52, 180} : Color{102, 44, 44, 180};
                DrawRectangleRec(slotRect, compareColor);
            } else if (hovered) {
                DrawRectangleRec(slotRect, Color{60, 74, 102, 170});
                if (equippedItem.has_value()) {
                    hoveredEquipItem = &equippedItem.value();
                }
            }

            if (hovered && equippedItem.has_value()) {
                hoveredEquipItem = &equippedItem.value();
            }

            std::string text = std::string(EquipSlotLabel(slot)) + ": ";
            text += equippedItem.has_value() ? equippedItem->name : "-";
            DrawText(text.c_str(), lineX, lineY, 20, equippedItem.has_value() ? Color{190, 225, 255, 255} : GRAY);

            if (hasEquipComparison && slot == comparisonSlot) {
                float centerX = slotRect.x + slotRect.width - 12.0f;
                float centerY = slotRect.y + slotRect.height * 0.5f;
                if (comparisonDirection > 0) {
                    Vector2 p1{centerX - 7.0f, centerY + 6.0f};
                    Vector2 p2{centerX + 7.0f, centerY + 6.0f};
                    Vector2 p3{centerX, centerY - 8.0f};
                    DrawTriangle(p1, p2, p3, Color{60, 220, 90, 255});
                } else if (comparisonDirection < 0) {
                    Vector2 p1{centerX + 7.0f, centerY - 6.0f};
                    Vector2 p2{centerX - 7.0f, centerY - 6.0f};
                    Vector2 p3{centerX, centerY + 8.0f};
                    DrawTriangle(p1, p2, p3, Color{255, 0, 0, 255});
                }
            }
            slotLine += 1;
        }

        DrawRectangle(listX, listY, listW, listH, Color{24, 24, 34, 220});
        DrawRectangleLines(listX, listY, listW, listH, Color{70, 70, 94, 255});
        DrawText("Backpack", listX + 10, listY + 8, 24, RAYWHITE);
        DrawText("Mouse Wheel: Scroll", listX + listW - 170, listY + 10, 18, GRAY);

        if (player.inventory.empty()) {
            DrawText("(Empty)", listX + 14, listY + 44, 22, GRAY);
            return;
        }

        BeginScissorMode(listX + 2, listY + 2, listW - 4, listH - 4);
        for (int row = 0; row < visibleRows; ++row) {
            for (int col = 0; col < kInventoryGridCols; ++col) {
                int index = (scrollRow + row) * kInventoryGridCols + col;
                if (index >= static_cast<int>(player.inventory.size())) continue;

                int cellX = listX + 10 + col * (kInventoryCellSize + kInventoryCellGap);
                int cellY = gridTop + row * (kInventoryCellSize + kInventoryCellGap);
                Rectangle cellRect{static_cast<float>(cellX), static_cast<float>(cellY), static_cast<float>(kInventoryCellSize), static_cast<float>(kInventoryCellSize)};
                bool hovered = CheckCollisionPointRec(mousePos, cellRect);
                if (hovered) hoveredInvIndex = index;

                const Item& cellItem = player.inventory[index];
                Color tierColor = ItemTierColor(cellItem.tier);
                bool selected = (index == inventorySelection);

                Color fill = Color{40, 40, 54, 220};
                if (selected) fill = Color{52, 72, 108, 220};
                else if (hovered) fill = Color{70, 82, 104, 170};

                DrawRectangleRec(cellRect, fill);
                DrawRectangleLinesEx(cellRect, 2.0f, tierColor);

                Rectangle iconSource{};
                if (TryGetItemIconSource(cellItem, iconSource)) {
                    const int iconDrawSize = kInventoryCellSize - 10;
                    Rectangle iconDest{
                        static_cast<float>(cellX + (kInventoryCellSize - iconDrawSize) / 2),
                        static_cast<float>(cellY + (kInventoryCellSize - iconDrawSize) / 2),
                        static_cast<float>(iconDrawSize),
                        static_cast<float>(iconDrawSize)
                    };
                    DrawTexturePro(iconAtlas, iconSource, iconDest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                } else {
                    const char* glyph = ItemGlyph(cellItem);
                    int glyphSize = 24;
                    int glyphW = MeasureText(glyph, glyphSize);
                    DrawText(glyph, cellX + (kInventoryCellSize - glyphW) / 2, cellY + 8, glyphSize, RAYWHITE);
                }

                if (cellItem.category == ItemCategory::Consumable && cellItem.quantity > 1) {
                    DrawText(TextFormat("%i", cellItem.quantity), cellX + kInventoryCellSize - 12, cellY + kInventoryCellSize - 16, 14, LIGHTGRAY);
                }

                if (cellItem.category == ItemCategory::Equipment && player.level < cellItem.itemLevel) {
                    DrawText("!", cellX + 4, cellY + 2, 20, RED);
                }
            }
        }
        EndScissorMode();

        const Item* tooltipItem = nullptr;
        if (hoveredInvIndex >= 0) {
            tooltipItem = &player.inventory[hoveredInvIndex];
        } else if (hoveredEquipItem != nullptr) {
            tooltipItem = hoveredEquipItem;
        }

        if (tooltipItem != nullptr) {
            Color tooltipBorder = ItemTierColor(tooltipItem->tier);
            struct TooltipLine {
                std::string text;
                int fontSize;
                Color color;
            };

            std::vector<TooltipLine> rawLines;
            rawLines.push_back({tooltipItem->name, 24, RAYWHITE});
            rawLines.push_back({ItemTierLabel(tooltipItem->tier), 22, tooltipBorder});
            rawLines.push_back({tooltipItem->category == ItemCategory::Equipment ? "Equipment" : "Consumable", 20, LIGHTGRAY});

            if (tooltipItem->category == ItemCategory::Equipment) {
                const char* slotLabel = (tooltipItem->slot == EquipSlot::Ring1 || tooltipItem->slot == EquipSlot::Ring2)
                    ? "Ring"
                    : EquipSlotLabel(tooltipItem->slot);
                rawLines.push_back({TextFormat("Slot: %s", slotLabel), 18, GRAY});
                if (tooltipItem->slot == EquipSlot::Weapon) {
                    rawLines.push_back({TextFormat("iLvl %i  Damage Modifier +%i", tooltipItem->itemLevel, static_cast<int>(std::round(tooltipItem->itemPower))), 18, GRAY});
                } else {
                    rawLines.push_back({TextFormat("iLvl %i  Defense Modifier +%i", tooltipItem->itemLevel, static_cast<int>(std::round(tooltipItem->itemPower))), 18, GRAY});
                }
                bool canEquip = player.level >= tooltipItem->itemLevel;
                rawLines.push_back({TextFormat("Required Level: %i", tooltipItem->itemLevel), 18, canEquip ? GREEN : RED});

                std::string stats;
                if (tooltipItem->strBonus != 0) stats += "STR+" + std::to_string(tooltipItem->strBonus) + " ";
                if (tooltipItem->dexBonus != 0) stats += "DEX+" + std::to_string(tooltipItem->dexBonus) + " ";
                if (tooltipItem->intBonus != 0) stats += "INT+" + std::to_string(tooltipItem->intBonus) + " ";
                if (tooltipItem->hpBonus != 0) stats += "HP+" + std::to_string(tooltipItem->hpBonus) + " ";
                if (tooltipItem->armorBonus != 0) stats += "ARM+" + std::to_string(tooltipItem->armorBonus) + " ";
                if (stats.empty()) stats = "No bonus stats";
                rawLines.push_back({stats, 18, LIGHTGRAY});
            } else {
                rawLines.push_back({TextFormat("Qty: %i", tooltipItem->quantity), 18, LIGHTGRAY});
                if (tooltipItem->consumableType == ConsumableType::HealthPotion) {
                    rawLines.push_back({"Effect: Restore 20 HP", 18, LIGHTGRAY});
                } else if (tooltipItem->consumableType == ConsumableType::ArmorPotion) {
                    rawLines.push_back({"Effect: +5 Armor", 18, LIGHTGRAY});
                } else if (tooltipItem->consumableType == ConsumableType::Antidote) {
                    rawLines.push_back({"Effect: Cure Poisoned", 18, LIGHTGRAY});
                } else if (tooltipItem->consumableType == ConsumableType::Bandage) {
                    rawLines.push_back({"Effect: Restore 10 HP, Cure Bleed", 18, LIGHTGRAY});
                }
                rawLines.push_back({"Consumables are Common rarity", 18, GRAY});
            }

            const int maxTooltipTextWidth = 520;
            std::vector<TooltipLine> lines;
            int contentWidth = 0;
            int contentHeight = 0;

            for (const TooltipLine& line : rawLines) {
                std::vector<std::string> wrapped = WrapTextStrict(line.text, maxTooltipTextWidth, line.fontSize);
                for (const std::string& segment : wrapped) {
                    lines.push_back({segment, line.fontSize, line.color});
                    contentWidth = std::max(contentWidth, MeasureText(segment.c_str(), line.fontSize));
                    contentHeight += line.fontSize + 4;
                }
            }

            const int padding = 10;
            int tooltipW = std::max(180, contentWidth + padding * 2);
            int tooltipH = std::max(80, contentHeight + padding * 2);

            int tooltipX = static_cast<int>(mousePos.x) + 18;
            int tooltipY = static_cast<int>(mousePos.y) + 18;
            if (tooltipX + tooltipW > kScreenWidth - 8) tooltipX = static_cast<int>(mousePos.x) - tooltipW - 18;
            if (tooltipY + tooltipH > kScreenHeight - 8) tooltipY = static_cast<int>(mousePos.y) - tooltipH - 18;
            if (tooltipX < 8) tooltipX = 8;
            if (tooltipY < 8) tooltipY = 8;

            DrawRectangle(tooltipX, tooltipY, tooltipW, tooltipH, Color{28, 28, 40, 235});
            DrawRectangleLinesEx(Rectangle{static_cast<float>(tooltipX), static_cast<float>(tooltipY), static_cast<float>(tooltipW), static_cast<float>(tooltipH)}, 2.0f, tooltipBorder);

            int textY = tooltipY + padding;
            for (const TooltipLine& line : lines) {
                DrawText(line.text.c_str(), tooltipX + padding, textY, line.fontSize, line.color);
                textY += line.fontSize + 4;
            }
        }
    }

    void DrawMerchantOverlay() const {
        DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.76f));

        const int panelX = 130;
        const int panelY = 64;
        const int panelW = kScreenWidth - 260;
        const int panelH = kScreenHeight - 128;
        const int listX = panelX + 18;
        const int listY = panelY + 132;
        const int listW = panelW - 36;
        const int listH = panelH - 156;

        DrawRectangle(panelX, panelY, panelW, panelH, Color{16, 18, 24, 248});
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{95, 84, 56, 255});

        DrawText("Merchant", panelX + 20, panelY + 14, 38, Color{255, 222, 130, 255});
        DrawText(TextFormat("Your Gold: %i", player.gold), panelX + panelW - 240, panelY + 24, 26, Color{255, 220, 120, 255});

        const MerchantTab tabs[3] = {MerchantTab::Buy, MerchantTab::Reroll, MerchantTab::Sell};
        for (int i = 0; i < 3; ++i) {
            MerchantTab tab = tabs[i];
            bool active = (tab == merchantTab);
            int tabX = panelX + 20 + i * 170;
            int tabY = panelY + 70;
            int tabW = 156;
            int tabH = 44;
            DrawRectangle(tabX, tabY, tabW, tabH, active ? Color{84, 70, 40, 220} : Color{34, 36, 44, 220});
            DrawRectangleLines(tabX, tabY, tabW, tabH, active ? Color{255, 220, 120, 255} : Color{82, 86, 96, 255});

            const char* label = MerchantTabLabel(tab);
            int tw = MeasureText(label, 24);
            DrawText(label, tabX + (tabW - tw) / 2, tabY + 10, 24, active ? Color{255, 230, 150, 255} : LIGHTGRAY);
        }

        DrawRectangle(listX, listY, listW, listH, Color{24, 26, 34, 225});
        DrawRectangleLines(listX, listY, listW, listH, Color{78, 82, 94, 255});

        std::vector<std::string> rows;
        std::vector<Color> rowColors;

        if (merchantTab == MerchantTab::Buy) {
            for (const Item& item : merchantStock) {
                int price = ComputeBuyValue(item);
                rows.push_back(item.name + "  -  " + std::to_string(price) + "g");
                rowColors.push_back(ItemTierColor(item.tier));
            }
            if (rows.empty()) {
                rows.push_back("Merchant is sold out.");
                rowColors.push_back(GRAY);
            }
        } else if (merchantTab == MerchantTab::Reroll) {
            std::vector<int> indices = MerchantInventoryIndices(true);
            for (int idx : indices) {
                const Item& item = player.inventory[idx];
                int price = ComputeRerollCost(item);
                rows.push_back(item.name + "  -  reroll " + std::to_string(price) + "g");
                rowColors.push_back(ItemTierColor(item.tier));
            }
            if (rows.empty()) {
                rows.push_back("No equipment in inventory.");
                rowColors.push_back(GRAY);
            }
        } else {
            std::vector<int> indices = MerchantInventoryIndices(false);
            for (int idx : indices) {
                const Item& item = player.inventory[idx];
                int value = ComputeSellValue(item);
                rows.push_back(item.name + "  -  sell " + std::to_string(value) + "g");
                rowColors.push_back(ItemTierColor(item.tier));
            }
            if (rows.empty()) {
                rows.push_back("Inventory is empty.");
                rowColors.push_back(GRAY);
            }
        }

        int maxVisible = std::max(1, (listH - 18) / 30);
        int selected = std::clamp(merchantSelection, 0, std::max(0, static_cast<int>(rows.size()) - 1));
        int start = std::clamp(selected - maxVisible / 2, 0, std::max(0, static_cast<int>(rows.size()) - maxVisible));
        int end = std::min(static_cast<int>(rows.size()), start + maxVisible);

        int rowY = listY + 10;
        for (int i = start; i < end; ++i) {
            bool hasSelection = !(rows.size() == 1 && (rows[0] == "Merchant is sold out." || rows[0] == "No equipment in inventory." || rows[0] == "Inventory is empty."));
            bool isSelected = hasSelection && (i == selected);
            if (isSelected) {
                DrawRectangle(listX + 8, rowY - 2, listW - 16, 28, Color{72, 78, 102, 170});
            }
            DrawText(rows[i].c_str(), listX + 14, rowY, 22, rowColors[i]);
            rowY += 30;
        }

        DrawText("Left/Right: Tab   Up/Down: Select   Enter/E: Confirm   Esc: Close", panelX + 20, panelY + panelH - 20, 20, LIGHTGRAY);
    }

    void DrawPauseMenu() const {
        DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.78f));

        const int panelW = 700;
        const int panelH = 470;
        const int panelX = (kScreenWidth - panelW) / 2;
        const int panelY = (kScreenHeight - panelH) / 2;

        DrawRectangle(panelX, panelY, panelW, panelH, Color{18, 20, 30, 248});
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{90, 100, 125, 255});

        DrawText("Menu", panelX + 24, panelY + 20, 42, RAYWHITE);
        DrawText("Press Esc to return", panelX + panelW - 250, panelY + 34, 22, LIGHTGRAY);

        int y = panelY + 90;
        DrawText("Controls", panelX + 24, y, 30, RAYWHITE); y += 44;
        DrawText("WASD / Arrows: Move", panelX + 28, y, 24, LIGHTGRAY); y += 34;
        DrawText("Mouse Left: Preview + Attack", panelX + 28, y, 24, LIGHTGRAY); y += 34;
        DrawText("I: Inventory", panelX + 28, y, 24, LIGHTGRAY); y += 34;
        DrawText("E: Interact", panelX + 28, y, 24, LIGHTGRAY); y += 34;
    }

    void DrawHud() const {
        const int hudX = kMapWidth * kTileSize;
        DrawRectangle(hudX, 0, kHudWidth, kScreenHeight, Color{14, 14, 20, 255});
        DrawLine(hudX, 0, hudX, kScreenHeight, Color{45, 45, 60, 255});

        int y = 24;
        DrawText("DONJON", hudX + 20, y, 32, RAYWHITE);
        y += 48;

        DrawText(TextFormat("Floor: %i", dungeonLevel), hudX + 20, y, 22, LIGHTGRAY); y += 30;
        Rectangle bagIcon{};
        if (TryGetGoldIconSource(3, bagIcon)) {
            Rectangle bagDst{static_cast<float>(hudX + 20), static_cast<float>(y), 22.0f, 22.0f};
            DrawTexturePro(iconAtlas, bagIcon, bagDst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
            DrawText(TextFormat("Gold: %i", player.gold), hudX + 50, y, 22, Color{255, 220, 120, 255});
        } else {
            DrawText(TextFormat("Gold: %i", player.gold), hudX + 20, y, 22, Color{255, 220, 120, 255});
        }
        y += 30;
        DrawText(TextFormat("HP: %i / %i", player.hp, player.maxHp), hudX + 20, y, 22, Color{255, 120, 120, 255}); y += 30;
        DrawText(TextFormat("ATK: %i", player.atk), hudX + 20, y, 22, LIGHTGRAY); y += 30;
        DrawText(TextFormat("DEF: %i", player.armor), hudX + 20, y, 22, LIGHTGRAY); y += 30;
        DrawText(TextFormat("Staff Charge: %i/5", player.staffCharge), hudX + 20, y, 22, LIGHTGRAY); y += 30;
        DrawText(TextFormat("LVL: %i", player.level), hudX + 20, y, 22, Color{130, 210, 255, 255}); y += 30;
        DrawText(TextFormat("XP: %i / %i", player.xp, player.xpToNext), hudX + 20, y, 22, LIGHTGRAY); y += 30;
        DrawText(TextFormat("Skill Pts: %i", player.skillPoints), hudX + 20, y, 22, Color{255, 220, 120, 255}); y += 32;
        DrawText("K: Skill Tree", hudX + 20, y, 20, LIGHTGRAY); y += 30;
        DrawText("Active skills: 1..0", hudX + 20, y, 20, Color{170, 205, 255, 255}); y += 28;

        DrawText("Passives", hudX + 20, y, 20, Color{200, 220, 255, 255}); y += 24;
        int shownPassiveCount = 0;
        int totalPassiveCount = static_cast<int>(player.passiveSkills.size());

        for (int i = std::max(0, totalPassiveCount - 3); i < totalPassiveCount; ++i) {
            const GeneratedPassive& passive = player.passiveSkills[static_cast<size_t>(i)];
            std::string line = PassiveSkillName(passive);
            int fontSize = 16;
            while (fontSize > 12 && MeasureText(line.c_str(), fontSize) > (kHudWidth - 62)) {
                fontSize -= 1;
            }

            Rectangle rarityChip{static_cast<float>(hudX + 20), static_cast<float>(y + 2), 12.0f, 12.0f};
            DrawRectangleRec(rarityChip, Fade(ItemTierColor(passive.grade), 0.35f));
            DrawRectangleLinesEx(rarityChip, 1.5f, ItemTierColor(passive.grade));

            DrawText(line.c_str(), hudX + 38, y, fontSize, Color{185, 200, 230, 255});
            y += 20;
            shownPassiveCount += 1;
        }
        if (shownPassiveCount == 0) {
            DrawText("None", hudX + 20, y, 18, Color{130, 140, 160, 255});
            y += 20;
        } else if (totalPassiveCount > shownPassiveCount) {
            int hiddenCount = totalPassiveCount - shownPassiveCount;
            DrawText(TextFormat("+%i more", hiddenCount), hudX + 20, y, 18, Color{150, 165, 195, 255});
            y += 20;
        }
        y += 8;

        if (CanInteractWithMerchant()) {
            DrawText("E: Talk to Merchant", hudX + 20, y, 20, Color{255, 220, 120, 255});
        }

        const int logX = hudX + 10;
        const int logY = std::max(340, y + 6);
        const int logW = kScreenWidth - logX - 10;
        const int logH = kScreenHeight - logY - 10;

        DrawRectangle(logX, logY, logW, logH, Fade(BLACK, 0.25f));
        DrawRectangleLines(logX, logY, logW, logH, DARKGRAY);

        std::vector<std::string> wrappedLines;
        for (const auto& msg : combatLog) {
            auto lines = WrapTextStrict(msg, logW - 12, 18);
            wrappedLines.insert(wrappedLines.end(), lines.begin(), lines.end());
        }

        const int lineHeight = 24; // slightly more spacing to avoid visual overlap
        const int maxVisibleLines = std::max(1, (logH - 12) / lineHeight);
        const int startLine = (wrappedLines.size() > static_cast<size_t>(maxVisibleLines))
            ? static_cast<int>(wrappedLines.size()) - maxVisibleLines
            : 0;

        BeginScissorMode(logX, logY, logW, logH);
        int lineY = logY + 6;
        for (int i = startLine; i < static_cast<int>(wrappedLines.size()); ++i) {
            DrawText(wrappedLines[i].c_str(), logX + 6, lineY, 18, LIGHTGRAY);
            lineY += lineHeight;
        }
        EndScissorMode();
    }
};

int main() {
    InitWindow(Game::kScreenWidth, Game::kScreenHeight, "DONJON - Roguelite Prototype");
    InitAudioDevice();
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    Game game;
    game.LoadAssets();
    game.StartNewRun();

    while (!WindowShouldClose()) {
        game.Update();

        BeginDrawing();
        ClearBackground(BLACK);
        game.Draw();
        EndDrawing();
    }

    game.UnloadAssets();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
