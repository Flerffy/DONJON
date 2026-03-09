#include "game.hpp"

#include "raylib.h"
#include "engine.hpp"
#include "util.hpp"

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
#include <fstream>

using donjon::util::WrapTextStrict;

enum class TileType : uint8_t {
    Outside,
    Wall,
    WallFloor,
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

enum class CharacterMenuTab : uint8_t {
    Inventory,
    Stats,
    SkillTree
};

enum class OptionsTab : uint8_t {
    Graphics,
    Audio,
    Controls,
    Back
};

enum class WindowModeSetting : uint8_t {
    Windowed,
    Fullscreen,
    Borderless
};

enum class OptionsReturnTarget : uint8_t {
    MainMenu,
    PauseMenu
};

enum class InteractableType : uint8_t {
    Chest,
    Pot,
    Button,
    Spike
};

enum class ActiveSkillId : uint8_t {
    None,
    WarriorCharge,
    WarriorCleave,
    WarriorBash,
    WarriorWhirlwind,
    WarriorDeathblow,
    WarriorPommelStrike,
    WarriorLunge,
    WarriorShout,
    WarriorGrapple,
    WarriorEnvironmentSmash,
    WarriorHunkerDown,
    WarriorParryRiposte,
    WarriorShieldWall,
    WarriorBodyguard,
    WarriorIndomitable,
    RangerSteadyAim,
    RangerPiercingBolt,
    RangerVolley,
    RangerCripplingShot,
    RangerAssassinate,
    RangerPointBlank,
    RangerScoutsEye,
    RangerVault,
    RangerTangleShot,
    RangerFlankingManeuver,
    RangerSmokeBomb,
    RangerCaltrops,
    RangerCamouflage,
    RangerDecoy,
    RangerCounterTrap,
    WizardMagicMissile,
    WizardFireball,
    WizardChainLightning,
    WizardArcaneBeam,
    WizardMeteor,
    WizardManaTap,
    WizardArcaneNova,
    WizardBlink,
    WizardTimeWarp,
    WizardTelekinesis,
    WizardArcaneEcho,
    WizardFrostNova,
    WizardManaShield,
    WizardMirrorImage,
    WizardRepulsionField,
    WizardStasis
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
    int mana = 5;
    bool isFlying = false;
    int stunnedTurns = 0;
    int grappledTurns = 0;
    int armorBreakTurns = 0;
    int armorBreakAmount = 0;
    bool facingRight = false;
    float attackBumpDx = 0.0f;
    float attackBumpDy = 0.0f;
    float attackBumpTimer = 0.0f;
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
    int mana = 5;
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
    int armedActiveSkillSlot = -1;
    int temporaryArmorBonus = 0;
    int temporaryArmorTurns = 0;
    int temporaryAttackBonus = 0;
    int temporaryAttackTurns = 0;
    int parryRiposteTurns = 0;
    bool hunkerDownReady = false;
    int unstoppableTurns = 0;
    bool knockbackImmune = false;
    int rangerSteadyAimCharges = 0;
    int rangerFlankingTurns = 0;
    int rangerCamouflageTurns = 0;
    bool rangerCounterTrapArmed = false;
    int rangerCounterTrapTurns = 0;
    int wizardArcaneEchoTurns = 0;
    int wizardManaShieldTurns = 0;
    int wizardMirrorImages = 0;
    int wizardRepulsionTurns = 0;
    int wizardStasisTurns = 0;
    int wizardArcaneBeamTurns = 0;
    int wizardArcaneBeamDx = 0;
    int wizardArcaneBeamDy = 0;
    int wizardLastSkillSlot = -1;
    bool wizardNextSpellManaTax = false;
    bool facingRight = false;
    float attackBumpDx = 0.0f;
    float attackBumpDy = 0.0f;
    float attackBumpTimer = 0.0f;
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

struct AttackParticle {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float life = 0.0f;
    float maxLife = 0.0f;
    float size = 2.0f;
    Color color = WHITE;
};

class Game {
public:
    static constexpr int kTileSize = 28;
    static constexpr int kMapWidth = 36;
    static constexpr int kMapHeight = 24;
    static constexpr int kScreenWidth = kMapWidth * kTileSize;
    static constexpr int kScreenHeight = kMapHeight * kTileSize;
    static constexpr int kMaxAttackParticles = 900;

    bool ShouldExitRequested() const {
        return exitRequested;
    }

    void LoadSettingsFromFile() {
        std::ifstream in("options.cfg");
        if (!in.is_open()) {
            SaveSettingsToFile();
            return;
        }

        std::string line;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            size_t eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);

            if (key == "resolution_index") {
                try {
                    selectedResolutionIndex = std::stoi(value);
                } catch (...) {
                }
            } else if (key == "window_mode") {
                if (value == "fullscreen") windowModeSetting = WindowModeSetting::Fullscreen;
                else if (value == "borderless") windowModeSetting = WindowModeSetting::Borderless;
                else windowModeSetting = WindowModeSetting::Windowed;
            } else if (key == "bgm") {
                try {
                    bgmVolumeLevel = std::stoi(value);
                } catch (...) {
                }
            } else if (key == "sfx") {
                try {
                    sfxVolumeLevel = std::stoi(value);
                } catch (...) {
                }
            }
        }

        selectedResolutionIndex = std::clamp(selectedResolutionIndex, 0, static_cast<int>(kResolutionOptions.size()) - 1);
        bgmVolumeLevel = std::clamp(bgmVolumeLevel, 0, 10);
        sfxVolumeLevel = std::clamp(sfxVolumeLevel, 0, 10);
    }

    void SaveSettingsToFile() const {
        std::ofstream out("options.cfg", std::ios::trunc);
        if (!out.is_open()) return;

        std::string mode = "windowed";
        if (windowModeSetting == WindowModeSetting::Fullscreen) mode = "fullscreen";
        if (windowModeSetting == WindowModeSetting::Borderless) mode = "borderless";

        out << "resolution_index=" << selectedResolutionIndex << "\n";
        out << "window_mode=" << mode << "\n";
        out << "bgm=" << bgmVolumeLevel << "\n";
        out << "sfx=" << sfxVolumeLevel << "\n";
    }

    void ApplyLoadedSettings() {
        selectedResolutionIndex = std::clamp(selectedResolutionIndex, 0, static_cast<int>(kResolutionOptions.size()) - 1);
        bgmVolumeLevel = std::clamp(bgmVolumeLevel, 0, 10);
        sfxVolumeLevel = std::clamp(sfxVolumeLevel, 0, 10);
        ApplyWindowModeSetting();
    }

    void LoadAssets() {
        engine::TryLoadTextureFromFile("assets/icons/room_atlas.png", roomAtlas, roomAtlasLoaded);
        engine::TryLoadTextureFromFile("assets/icons/action_atlas.png", actionAtlas, actionAtlasLoaded);
        engine::TryLoadTextureFromFile("assets/icons/walkables_atlas.png", walkablesAtlas, walkablesAtlasLoaded);
        engine::TryLoadTextureFromFile("assets/icons/solids_atlas.png", solidsAtlas, solidsAtlasLoaded);
        engine::TryLoadTextureFromFile("assets/icons/stairs.png", stairsIcon, stairsIconLoaded);
        engine::TryLoadTextureFromFile("assets/icons/item_atlas.png", iconAtlas, iconAtlasLoaded);
        engine::TryLoadTextureFromFile("assets/icons/player_atlas.png", playerAtlas, playerAtlasLoaded);
        engine::TryLoadTextureFromFile("assets/icons/enemy_atlas.png", enemyAtlas, enemyAtlasLoaded);
        engine::TryLoadTextureFromFile("assets/icons/npc_atlas.png", npcAtlas, npcAtlasLoaded);

        for (size_t i = 1; i < kActiveSkillIconCount; ++i) {
            if (activeSkillIconsLoaded[i]) continue;
            ActiveSkillId skillId = static_cast<ActiveSkillId>(i);
            std::string path = ActiveSkillIconPath(skillId);
            if (path.empty()) continue;
            engine::TryLoadTextureFromFile(path.c_str(), activeSkillIcons[i], activeSkillIconsLoaded[i]);
        }

        for (size_t i = 0; i < kPassiveIconCount; ++i) {
            if (passiveIconsLoaded[i]) continue;
            PassiveSuffixId suffixId = static_cast<PassiveSuffixId>(i);
            std::string path = PassiveIconPath(suffixId);
            if (path.empty()) continue;
            engine::TryLoadTextureFromFile(path.c_str(), passiveIcons[i], passiveIconsLoaded[i]);
        }

        if (!musicLoaded) {
            const char* bgmPath = FileExists("assets/audio/Goblins_Den_(Regular).wav")
                ? "assets/audio/Goblins_Den_(Regular).wav"
                : (FileExists("assets/audio/bgm.ogg") ? "assets/audio/bgm.ogg" : nullptr);
            if (bgmPath != nullptr && engine::TryLoadMusicFromFile(bgmPath, musicBgm, musicLoaded)) {
                SetMusicVolume(musicBgm, static_cast<float>(bgmVolumeLevel) / 10.0f);
                PlayMusicStream(musicBgm);
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
        engine::SafeUnloadTexture(roomAtlas, roomAtlasLoaded);
        engine::SafeUnloadTexture(actionAtlas, actionAtlasLoaded);
        engine::SafeUnloadTexture(walkablesAtlas, walkablesAtlasLoaded);
        engine::SafeUnloadTexture(solidsAtlas, solidsAtlasLoaded);
        engine::SafeUnloadTexture(stairsIcon, stairsIconLoaded);
        engine::SafeUnloadTexture(iconAtlas, iconAtlasLoaded);
        engine::SafeUnloadTexture(playerAtlas, playerAtlasLoaded);
        engine::SafeUnloadTexture(enemyAtlas, enemyAtlasLoaded);
        engine::SafeUnloadTexture(npcAtlas, npcAtlasLoaded);

        for (size_t i = 0; i < kActiveSkillIconCount; ++i) {
            if (!activeSkillIconsLoaded[i]) continue;
            engine::SafeUnloadTexture(activeSkillIcons[i], activeSkillIconsLoaded[i]);
        }

        for (size_t i = 0; i < kPassiveIconCount; ++i) {
            if (!passiveIconsLoaded[i]) continue;
            engine::SafeUnloadTexture(passiveIcons[i], passiveIconsLoaded[i]);
        }

        engine::SafeUnloadMusic(musicBgm, musicLoaded);

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

        mainMenuOpen = false;
        classSelectionPending = true;
        mainMenuSelection = 0;
        characterMenuOpen = false;
        characterMenuTab = CharacterMenuTab::Inventory;
        inventorySelection = 0;
        inventoryScrollOffset = 0;
        statsAttributeSelection = 0;
        pauseMenuOpen = false;
        pauseMenuSelection = 0;
        optionsMenuOpen = false;
        optionsTab = OptionsTab::Graphics;
        devConsoleOpen = false;
        devConsoleInput.clear();

        // Keep render-safe data before class is chosen
        tiles.assign(kMapWidth * kMapHeight, TileType::Outside);
        enemies.clear();
        goldStacks.clear();
        walkableDecorations.clear();
        solidDecorations.clear();
        interactables.clear();
        attackParticles.clear();
        rooms.clear();
        passiveChoiceOpen = false;
        passiveChoiceSelection = 0;
        pendingPassiveChoices = 0;
        passiveChoiceOptions = {GeneratedPassive{}, GeneratedPassive{}, GeneratedPassive{}};
        cameraTargetInitialized = false;
        cameraSmoothedTarget = Vector2{0.0f, 0.0f};

        AddLog("Choose class: [1] Warrior  [2] Ranger  [3] Wizard");
    }

    void ReturnToMainMenu() {
        mainMenuOpen = true;
        classSelectionPending = false;
        pauseMenuOpen = false;
        optionsMenuOpen = false;
        merchantOpen = false;
        characterMenuOpen = false;
        passiveChoiceOpen = false;
        player.armedActiveSkillSlot = -1;
        devConsoleOpen = false;
        mainMenuSelection = 0;
    }

    const char* OptionsTabLabel(OptionsTab tab) const {
        switch (tab) {
        case OptionsTab::Graphics: return "Graphics";
        case OptionsTab::Audio: return "Audio";
        case OptionsTab::Controls: return "Controls";
        case OptionsTab::Back: return "Back";
        default: return "Graphics";
        }
    }

    const char* WindowModeLabel(WindowModeSetting mode) const {
        switch (mode) {
        case WindowModeSetting::Windowed: return "Windowed";
        case WindowModeSetting::Fullscreen: return "Fullscreen";
        case WindowModeSetting::Borderless: return "Borderless Window";
        default: return "Windowed";
        }
    }

    void OpenOptionsMenu(OptionsReturnTarget target) {
        optionsMenuOpen = true;
        optionsReturnTarget = target;
        optionsTab = OptionsTab::Graphics;
    }

    void CloseOptionsMenu() {
        optionsMenuOpen = false;
        if (optionsReturnTarget == OptionsReturnTarget::PauseMenu) {
            pauseMenuOpen = true;
        }
    }

    void ApplySelectedResolution() {
        selectedResolutionIndex = std::clamp(selectedResolutionIndex, 0, static_cast<int>(kResolutionOptions.size()) - 1);
        ResolutionOption option = kResolutionOptions[static_cast<size_t>(selectedResolutionIndex)];
        SetWindowSize(option.width, option.height);
    }

    void ApplyWindowModeSetting() {
        if (windowModeSetting == WindowModeSetting::Fullscreen) {
            if (!IsWindowFullscreen()) {
                ToggleFullscreen();
            }
            return;
        }

        if (IsWindowFullscreen()) {
            ToggleFullscreen();
        }

        if (windowModeSetting == WindowModeSetting::Borderless) {
            SetWindowState(FLAG_WINDOW_UNDECORATED);
            int monitor = GetCurrentMonitor();
            SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
            SetWindowPosition(0, 0);
        } else {
            ClearWindowState(FLAG_WINDOW_UNDECORATED);
            ApplySelectedResolution();
        }
    }

    void HandleMainMenuInput() {
        const int panelW = 520;
        const int panelH = 380;
        const int panelX = (kScreenWidth - panelW) / 2;
        const int panelY = (kScreenHeight - panelH) / 2;
        const int buttonW = 360;
        const int buttonH = 52;
        const int buttonX = panelX + (panelW - buttonW) / 2;
        const int firstButtonY = panelY + 120;

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            mainMenuSelection = (mainMenuSelection + 2) % 3;
            PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            mainMenuSelection = (mainMenuSelection + 1) % 3;
            PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
        }

        Vector2 mousePos = GetMousePosition();
        for (int i = 0; i < 3; ++i) {
            Rectangle rect{static_cast<float>(buttonX), static_cast<float>(firstButtonY + i * 64), static_cast<float>(buttonW), static_cast<float>(buttonH)};
            if (CheckCollisionPointRec(mousePos, rect)) {
                mainMenuSelection = i;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (i == 0) {
                        PlaySfx(sfxInteract, sfxInteractLoaded);
                        StartNewRun();
                    } else if (i == 1) {
                        PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
                        OpenOptionsMenu(OptionsReturnTarget::MainMenu);
                    } else {
                        exitRequested = true;
                    }
                }
            }
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            if (mainMenuSelection == 0) {
                PlaySfx(sfxInteract, sfxInteractLoaded);
                StartNewRun();
            } else if (mainMenuSelection == 1) {
                PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
                OpenOptionsMenu(OptionsReturnTarget::MainMenu);
            } else {
                exitRequested = true;
            }
        }
    }

    void HandlePauseMenuInput() {
        const int panelW = 560;
        const int panelH = 420;
        const int panelX = (kScreenWidth - panelW) / 2;
        const int panelY = (kScreenHeight - panelH) / 2;
        const int buttonW = 360;
        const int buttonH = 52;
        const int buttonX = panelX + (panelW - buttonW) / 2;
        const int firstButtonY = panelY + 100;

        if (IsKeyPressed(KEY_ESCAPE)) {
            pauseMenuOpen = false;
            PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            return;
        }

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            pauseMenuSelection = (pauseMenuSelection + 3) % 4;
            PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            pauseMenuSelection = (pauseMenuSelection + 1) % 4;
            PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
        }

        auto activateSelection = [&](int selection) {
            if (selection == 0) {
                pauseMenuOpen = false;
                PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            } else if (selection == 1) {
                PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
                pauseMenuOpen = false;
                OpenOptionsMenu(OptionsReturnTarget::PauseMenu);
            } else if (selection == 2) {
                PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
                ReturnToMainMenu();
            } else {
                exitRequested = true;
            }
        };

        Vector2 mousePos = GetMousePosition();
        for (int i = 0; i < 4; ++i) {
            Rectangle rect{static_cast<float>(buttonX), static_cast<float>(firstButtonY + i * 64), static_cast<float>(buttonW), static_cast<float>(buttonH)};
            if (CheckCollisionPointRec(mousePos, rect)) {
                pauseMenuSelection = i;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    activateSelection(i);
                    return;
                }
            }
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            activateSelection(pauseMenuSelection);
        }
    }

    void HandleOptionsMenuInput() {
        const int panelW = 900;
        const int panelH = 560;
        const int panelX = (kScreenWidth - panelW) / 2;
        const int panelY = (kScreenHeight - panelH) / 2;

        if (IsKeyPressed(KEY_ESCAPE)) {
            PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            CloseOptionsMenu();
            return;
        }

        Vector2 mousePos = GetMousePosition();
        for (int i = 0; i < 4; ++i) {
            Rectangle tabRect{static_cast<float>(panelX + 24 + i * 210), static_cast<float>(panelY + 74), 190.0f, 42.0f};
            if (!CheckCollisionPointRec(mousePos, tabRect)) continue;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                OptionsTab clickedTab = static_cast<OptionsTab>(i);
                if (clickedTab == OptionsTab::Back) {
                    PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
                    CloseOptionsMenu();
                    return;
                }
                optionsTab = clickedTab;
                PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
                return;
            }
        }

        if (optionsTab == OptionsTab::Graphics) {
            Rectangle resLeft{static_cast<float>(panelX + 90), static_cast<float>(panelY + 190), 40.0f, 40.0f};
            Rectangle resRight{static_cast<float>(panelX + 540), static_cast<float>(panelY + 190), 40.0f, 40.0f};
            Rectangle modeLeft{static_cast<float>(panelX + 90), static_cast<float>(panelY + 270), 40.0f, 40.0f};
            Rectangle modeRight{static_cast<float>(panelX + 540), static_cast<float>(panelY + 270), 40.0f, 40.0f};

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mousePos, resLeft)) {
                    selectedResolutionIndex = (selectedResolutionIndex - 1 + static_cast<int>(kResolutionOptions.size())) % static_cast<int>(kResolutionOptions.size());
                    ApplySelectedResolution();
                    SaveSettingsToFile();
                    PlaySfx(sfxInteract, sfxInteractLoaded);
                    return;
                }
                if (CheckCollisionPointRec(mousePos, resRight)) {
                    selectedResolutionIndex = (selectedResolutionIndex + 1) % static_cast<int>(kResolutionOptions.size());
                    ApplySelectedResolution();
                    SaveSettingsToFile();
                    PlaySfx(sfxInteract, sfxInteractLoaded);
                    return;
                }
                if (CheckCollisionPointRec(mousePos, modeLeft)) {
                    int mode = static_cast<int>(windowModeSetting) - 1;
                    if (mode < 0) mode = 2;
                    windowModeSetting = static_cast<WindowModeSetting>(mode);
                    ApplyWindowModeSetting();
                    SaveSettingsToFile();
                    PlaySfx(sfxInteract, sfxInteractLoaded);
                    return;
                }
                if (CheckCollisionPointRec(mousePos, modeRight)) {
                    int mode = static_cast<int>(windowModeSetting) + 1;
                    if (mode > 2) mode = 0;
                    windowModeSetting = static_cast<WindowModeSetting>(mode);
                    ApplyWindowModeSetting();
                    SaveSettingsToFile();
                    PlaySfx(sfxInteract, sfxInteractLoaded);
                    return;
                }
            }
        } else if (optionsTab == OptionsTab::Audio) {
            Rectangle bgmBar{static_cast<float>(panelX + 180), static_cast<float>(panelY + 190), 320.0f, 20.0f};
            Rectangle sfxBar{static_cast<float>(panelX + 180), static_cast<float>(panelY + 270), 320.0f, 20.0f};

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mousePos, bgmBar)) {
                    float t = (mousePos.x - bgmBar.x) / bgmBar.width;
                    bgmVolumeLevel = std::clamp(static_cast<int>(std::round(t * 10.0f)), 0, 10);
                    SaveSettingsToFile();
                }
                if (CheckCollisionPointRec(mousePos, sfxBar)) {
                    float t = (mousePos.x - sfxBar.x) / sfxBar.width;
                    sfxVolumeLevel = std::clamp(static_cast<int>(std::round(t * 10.0f)), 0, 10);
                    SaveSettingsToFile();
                }
            }
        }
    }

    void Update(const engine::FrameContext& frame) {
        turnClock.TickFrame(frame.deltaTime);
        const engine::InputSnapshot input = engine::PollInput();

        UpdateAudio();
        TickAfflictionFlashes();
        TickAttackBumpAnimations();
        TickAttackParticles();

        if (mainMenuOpen) {
            if (optionsMenuOpen) {
                HandleOptionsMenuInput();
            } else {
                HandleMainMenuInput();
            }
            return;
        }

        // Class selection gate
        if (classSelectionPending) {
            if (input.skillSlotPressed[0]) {
                PlaySfx(sfxInteract, sfxInteractLoaded);
                ApplyClassPreset(PlayerClass::Warrior);
            } else if (input.skillSlotPressed[1]) {
                PlaySfx(sfxInteract, sfxInteractLoaded);
                ApplyClassPreset(PlayerClass::Ranger);
            } else if (input.skillSlotPressed[2]) {
                PlaySfx(sfxInteract, sfxInteractLoaded);
                ApplyClassPreset(PlayerClass::Wizard);
            } else if (input.mouseLeftPressed) {
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
            if (input.restartPressed) {
                StartNewRun();
            }
            return;
        }

        if (input.consoleTogglePressed) {
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

        UpdateCameraRig();

        auto closeAllMenus = [&]() {
            merchantOpen = false;
            characterMenuOpen = false;
            pauseMenuOpen = false;
            player.armedActiveSkillSlot = -1;
        };

        if (input.escapePressed) {
            if (optionsMenuOpen) {
                CloseOptionsMenu();
                PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            } else if (merchantOpen || characterMenuOpen || pauseMenuOpen) {
                closeAllMenus();
                PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            } else {
                pauseMenuOpen = true;
                pauseMenuSelection = 0;
                PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
            }
            return;
        }

        if (input.tabPressed) {
            bool willOpen = !characterMenuOpen;
            closeAllMenus();
            characterMenuOpen = willOpen;
            if (characterMenuOpen) {
                characterMenuTab = CharacterMenuTab::Inventory;
                PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
                ClampInventorySelection();
            } else {
                PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            }
            return;
        }

        TryOpenMerchantRoom();

        if (merchantOpen) {
            HandleMerchantInput();
            return;
        }

        if (input.interactPressed && TryInteractWithNearbyActions()) {
            if (player.hunkerDownReady) {
                player.hunkerDownReady = false;
                player.knockbackImmune = false;
                player.temporaryArmorBonus = 0;
                player.temporaryArmorTurns = 0;
                RecomputePlayerStats();
            }
            turnClock.AdvanceTurn();
            TickActiveSkillStateForTurn();
            CheckGameOver();
            if (!gameOver) {
                EnemyTurn();
                CheckGameOver();
            }
            return;
        }

        if (input.interactPressed && CanInteractWithMerchant()) {
            PlaySfx(sfxInteract, sfxInteractLoaded);
            closeAllMenus();
            merchantOpen = true;
            merchantTab = MerchantTab::Buy;
            merchantSelection = 0;
            return;
        }

        if (optionsMenuOpen) {
            HandleOptionsMenuInput();
            return;
        }

        if (pauseMenuOpen) {
            HandlePauseMenuInput();
            return;
        }

        if (characterMenuOpen) {
            HandleCharacterMenuInput();
            return;
        }

        int activeSkillSlot = GetPressedActiveSkillSlot(input);
        if (activeSkillSlot >= 0) {
            ActiveSkillId chosenSkill = player.activeSkillLoadout[static_cast<size_t>(activeSkillSlot)];
            if (chosenSkill == ActiveSkillId::None) {
                AddLog("No skill bound to that slot.");
            } else if (player.armedActiveSkillSlot == activeSkillSlot) {
                player.armedActiveSkillSlot = -1;
                PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            } else {
                player.armedActiveSkillSlot = activeSkillSlot;
                PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
            }
            return;
        }

        if (player.armedActiveSkillSlot >= 0 && input.mouseRightPressed) {
            player.armedActiveSkillSlot = -1;
            PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            AddLog("Skill aiming canceled.");
            return;
        }

        if (input.mouseLeftPressed) {
            if (player.armedActiveSkillSlot >= 0) {
                std::vector<std::pair<int, int>> preview = GetActiveSkillPreviewTiles(player.armedActiveSkillSlot);
                if (preview.empty()) {
                    PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
                    return;
                }
                if (TryUseActiveSkillSlot(player.armedActiveSkillSlot)) {
                    player.armedActiveSkillSlot = -1;
                    turnClock.AdvanceTurn();
                    TickActiveSkillStateForTurn();
                    CheckGameOver();
                    if (!gameOver) {
                        EnemyTurn();
                        CheckGameOver();
                    }
                }
                return;
            }
            if (TryMouseWeaponAttack()) {
                if (player.hunkerDownReady) {
                    player.hunkerDownReady = false;
                    player.knockbackImmune = false;
                    player.temporaryArmorBonus = 0;
                    player.temporaryArmorTurns = 0;
                    RecomputePlayerStats();
                }
                turnClock.AdvanceTurn();
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

        switch (engine::ReadCardinalMove(input)) {
        case engine::MoveIntent::Up:
            consumedTurn = PlayerTryMove(0, -1, changedFloor);
            break;
        case engine::MoveIntent::Down:
            consumedTurn = PlayerTryMove(0, 1, changedFloor);
            break;
        case engine::MoveIntent::Left:
            consumedTurn = PlayerTryMove(-1, 0, changedFloor);
            break;
        case engine::MoveIntent::Right:
            consumedTurn = PlayerTryMove(1, 0, changedFloor);
            break;
        case engine::MoveIntent::None:
            break;
        }

        CheckGameOver();
        if (gameOver) return;

        if (consumedTurn && (player.x != oldPlayerX || player.y != oldPlayerY)) {
            player.mana = std::min(PlayerMaxMana(), player.mana + 1);
            if (player.rangerCamouflageTurns > 0) {
                player.rangerCamouflageTurns = 0;
                AddLog("Camouflage breaks as you move.");
            }
        }

        if (consumedTurn) {
            turnClock.AdvanceTurn();
            if (player.hunkerDownReady) {
                player.hunkerDownReady = false;
                player.knockbackImmune = false;
                player.temporaryArmorBonus = 0;
                player.temporaryArmorTurns = 0;
                RecomputePlayerStats();
            }
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
        if (mainMenuOpen) {
            DrawMainMenu();
            if (optionsMenuOpen) {
                DrawOptionsMenu();
            }
            return;
        }

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
        DrawAttackParticles();
        EndMode2D();
        EndScissorMode();
        DrawMapStatusOutline();
        DrawActiveSkillHotbar();
        DrawHud();

        if (devConsoleOpen) {
            DrawDevConsole();
        }

        if (merchantOpen) {
            DrawMerchantOverlay();
        }

        if (characterMenuOpen) {
            DrawCharacterMenuOverlay();
        }

        if (passiveChoiceOpen) {
            DrawPassiveChoiceOverlay();
        }

        if (pauseMenuOpen) {
            DrawPauseMenu();
        }

        if (optionsMenuOpen) {
            DrawOptionsMenu();
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
    struct ResolutionOption {
        int width;
        int height;
        const char* label;
    };

    static constexpr int kEnemyViewRange = 8;
    static constexpr float kMapCameraZoom = 1.85f;
    static constexpr float kCameraMouseShiftWorldPctX = 0.40f;
    static constexpr float kCameraMouseShiftWorldPctY = 0.34f;
    static constexpr float kCameraPlayerSafeMarginPxX = 120.0f;
    static constexpr float kCameraPlayerSafeMarginPxY = 96.0f;
    static constexpr float kCameraPanSmoothing = 4.8f;
    static constexpr float kEntityRockAngleDeg = 3.2f;
    static constexpr float kEntityRockSpeed = 2.25f;
    static constexpr float kEntityBouncePx = 0.85f;
    static constexpr float kEntityBounceSpeed = 5.2f;
    static constexpr float kEntityShufflePx = 1.5f;
    static constexpr float kFlyingFloatPx = 3.2f;
    static constexpr float kFlyingFloatSpeed = 2.7f;
    static constexpr float kAttackBumpDuration = 0.14f;
    static constexpr float kAttackBumpDistancePx = 6.0f;
    static constexpr float kAfflictionFlashDuration = 0.16f;
    static constexpr int kClassSkillCount = 3;
    static constexpr int kClassSkillMaxRank = 5;
    static constexpr size_t kActiveSkillIconCount = static_cast<size_t>(ActiveSkillId::WizardStasis) + 1;
    static constexpr size_t kPassiveIconCount = static_cast<size_t>(PassiveSuffixId::Radiance) + 1;
    static constexpr std::array<ResolutionOption, 10> kResolutionOptions{{
        {640, 480, "640x480 (4:3)"},
        {800, 600, "800x600 (4:3)"},
        {1024, 768, "1024x768 (4:3)"},
        {1280, 960, "1280x960 (4:3)"},
        {1280, 720, "1280x720 (16:9)"},
        {1366, 768, "1366x768 (16:9)"},
        {1600, 900, "1600x900 (16:9)"},
        {1920, 1080, "1920x1080 (16:9)"},
        {2560, 1440, "2560x1440 (16:9)"},
        {3840, 2160, "3840x2160 (16:9)"}
    }};

    std::vector<TileType> tiles;
    std::vector<Enemy> enemies;
    std::vector<GoldStack> goldStacks;
    std::vector<Decoration> walkableDecorations;
    std::vector<Decoration> solidDecorations;
    std::vector<Interactable> interactables;
    std::vector<AttackParticle> attackParticles;
    std::vector<Room> rooms;
    std::vector<Item> merchantStock;
    std::vector<std::string> combatLog;
    Player player;
    int dungeonLevel = 1;
    int nextMerchantFloor = 4;
    int score = 0;
    bool gameOver = false;
    bool exitRequested = false;
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
    engine::Rng rng{};
    engine::TurnClock turnClock{};
    bool classSelectionPending = false;
    bool mainMenuOpen = true;
    int mainMenuSelection = 0;
    bool characterMenuOpen = false;
    CharacterMenuTab characterMenuTab = CharacterMenuTab::Inventory;
    int inventorySelection = 0;
    int inventoryScrollOffset = 0; // row offset for grid scrolling
    int statsAttributeSelection = 0;
    int skillSelection = 0;
    int skillRankSelection = 1;
    bool passiveChoiceOpen = false;
    int passiveChoiceSelection = 0;
    int pendingPassiveChoices = 0;
    std::array<GeneratedPassive, 3> passiveChoiceOptions{};
    Vector2 cameraSmoothedTarget = Vector2{0.0f, 0.0f};
    bool cameraTargetInitialized = false;
    bool pauseMenuOpen = false;
    int pauseMenuSelection = 0;
    bool optionsMenuOpen = false;
    OptionsTab optionsTab = OptionsTab::Graphics;
    OptionsReturnTarget optionsReturnTarget = OptionsReturnTarget::MainMenu;
    WindowModeSetting windowModeSetting = WindowModeSetting::Windowed;
    int selectedResolutionIndex = 4;
    int bgmVolumeLevel = 5;
    int sfxVolumeLevel = 6;
    bool devConsoleOpen = false;
    std::string devConsoleInput;
    std::vector<std::string> devConsoleHistory;
    float poisonAfflictFlashTimer = 0.0f;
    float bleedAfflictFlashTimer = 0.0f;
    static constexpr int kInventoryGridCols = 8;
    static constexpr int kInventoryCellSize = 44;
    static constexpr int kInventoryCellGap = 6;
    static constexpr int kItemIconSize = 32;
    static constexpr int kSkillTreeSkillCap = 10;
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
    std::array<Texture2D, kActiveSkillIconCount> activeSkillIcons{};
    std::array<bool, kActiveSkillIconCount> activeSkillIconsLoaded{};
    std::array<Texture2D, kPassiveIconCount> passiveIcons{};
    std::array<bool, kPassiveIconCount> passiveIconsLoaded{};
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
        engine::TryLoadSoundFromFile(path, sound, loaded, volume);
    }

    void UnloadSfx(Sound& sound, bool& loaded) {
        engine::SafeUnloadSound(sound, loaded);
    }

    void PlaySfx(Sound& sound, bool loaded, bool varyPitch = false, float pitchMin = 0.96f, float pitchMax = 1.04f) {
        if (!loaded) return;
        SetSoundVolume(sound, static_cast<float>(sfxVolumeLevel) / 10.0f);
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
        SetMusicVolume(musicBgm, static_cast<float>(bgmVolumeLevel) / 10.0f);
        UpdateMusicStream(musicBgm);
        if (!IsMusicStreamPlaying(musicBgm)) {
            PlayMusicStream(musicBgm);
        }
    }

    Vector2 ComputeDesiredCameraTarget() const {
        const float mapViewWidth = static_cast<float>(kMapWidth * kTileSize);
        const float mapViewHeight = static_cast<float>(kMapHeight * kTileSize);
        const float worldWidth = static_cast<float>(kMapWidth * kTileSize);
        const float worldHeight = static_cast<float>(kMapHeight * kTileSize);
        const float zoom = kMapCameraZoom;

        const float playerWorldX = static_cast<float>(player.x * kTileSize + kTileSize / 2);
        const float playerWorldY = static_cast<float>(player.y * kTileSize + kTileSize / 2);

        Vector2 mousePos = GetMousePosition();
        mousePos.x = std::clamp(mousePos.x, 0.0f, mapViewWidth - 1.0f);
        mousePos.y = std::clamp(mousePos.y, 0.0f, mapViewHeight - 1.0f);

        const float offsetX = mapViewWidth * 0.5f;
        const float offsetY = mapViewHeight * 0.5f;
        float normalizedX = std::clamp((mousePos.x - offsetX) / std::max(1.0f, offsetX), -1.0f, 1.0f);
        float normalizedY = std::clamp((mousePos.y - offsetY) / std::max(1.0f, offsetY), -1.0f, 1.0f);

        float easedX = engine::SmoothStep01(std::abs(normalizedX));
        float easedY = engine::SmoothStep01(std::abs(normalizedY));

        float maxShiftX = (mapViewWidth * 0.5f / zoom) * kCameraMouseShiftWorldPctX;
        float maxShiftY = (mapViewHeight * 0.5f / zoom) * kCameraMouseShiftWorldPctY;
        float shiftX = (normalizedX >= 0.0f ? 1.0f : -1.0f) * maxShiftX * easedX;
        float shiftY = (normalizedY >= 0.0f ? 1.0f : -1.0f) * maxShiftY * easedY;

        float targetX = playerWorldX + shiftX;
        float targetY = playerWorldY + shiftY;

        float maxPlayerOffsetX = std::max(0.0f, (mapViewWidth * 0.5f - kCameraPlayerSafeMarginPxX) / zoom);
        float maxPlayerOffsetY = std::max(0.0f, (mapViewHeight * 0.5f - kCameraPlayerSafeMarginPxY) / zoom);
        targetX = std::clamp(targetX, playerWorldX - maxPlayerOffsetX, playerWorldX + maxPlayerOffsetX);
        targetY = std::clamp(targetY, playerWorldY - maxPlayerOffsetY, playerWorldY + maxPlayerOffsetY);

        float halfViewWorldW = mapViewWidth * 0.5f / zoom;
        float halfViewWorldH = mapViewHeight * 0.5f / zoom;

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

        return Vector2{targetX, targetY};
    }

    void UpdateCameraRig() {
        Vector2 desiredTarget = ComputeDesiredCameraTarget();
        if (!cameraTargetInitialized) {
            cameraSmoothedTarget = desiredTarget;
            cameraTargetInitialized = true;
            return;
        }

        float dt = GetFrameTime();
        float alpha = engine::ExponentialSmoothingAlpha(kCameraPanSmoothing, dt);
        cameraSmoothedTarget = engine::LerpVector2(cameraSmoothedTarget, desiredTarget, alpha);
    }

    Camera2D BuildPlayerCamera() const {
        const float mapViewWidth = static_cast<float>(kMapWidth * kTileSize);
        const float mapViewHeight = static_cast<float>(kMapHeight * kTileSize);

        Camera2D camera{};
        camera.offset = Vector2{mapViewWidth * 0.5f, mapViewHeight * 0.5f};
        camera.rotation = 0.0f;
        camera.zoom = kMapCameraZoom;

        if (cameraTargetInitialized) {
            camera.target = cameraSmoothedTarget;
        } else {
            camera.target = ComputeDesiredCameraTarget();
        }
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

    void TickAttackBumpAnimations() {
        float dt = GetFrameTime();
        player.attackBumpTimer = std::max(0.0f, player.attackBumpTimer - dt);
        for (Enemy& enemy : enemies) {
            enemy.attackBumpTimer = std::max(0.0f, enemy.attackBumpTimer - dt);
        }
    }

    void EmitAttackParticlesAtTile(int tileX, int tileY, Color baseColor, int count) {
        if (count <= 0) return;
        const float centerX = static_cast<float>(tileX * kTileSize) + static_cast<float>(kTileSize) * 0.5f;
        const float centerY = static_cast<float>(tileY * kTileSize) + static_cast<float>(kTileSize) * 0.5f;

        std::uniform_real_distribution<float> angleDist(0.0f, 6.2831853f);
        std::uniform_real_distribution<float> speedDist(38.0f, 140.0f);
        std::uniform_real_distribution<float> lifeDist(0.12f, 0.34f);
        std::uniform_real_distribution<float> jitterDist(-4.0f, 4.0f);
        std::uniform_real_distribution<float> sizeDist(1.0f, 3.0f);

        for (int i = 0; i < count; ++i) {
            float angle = angleDist(rng);
            float speed = speedDist(rng);
            float life = lifeDist(rng);

            AttackParticle particle;
            particle.x = centerX + jitterDist(rng);
            particle.y = centerY + jitterDist(rng);
            particle.vx = std::cos(angle) * speed;
            particle.vy = std::sin(angle) * speed;
            particle.life = life;
            particle.maxLife = life;
            particle.size = sizeDist(rng);
            particle.color = baseColor;
            attackParticles.push_back(particle);
        }

        if (attackParticles.size() > static_cast<size_t>(kMaxAttackParticles)) {
            size_t overflow = attackParticles.size() - static_cast<size_t>(kMaxAttackParticles);
            attackParticles.erase(attackParticles.begin(), attackParticles.begin() + static_cast<std::ptrdiff_t>(overflow));
        }
    }

    void TickAttackParticles() {
        float dt = GetFrameTime();
        if (dt <= 0.0f || attackParticles.empty()) return;

        for (AttackParticle& particle : attackParticles) {
            particle.life -= dt;
            if (particle.life <= 0.0f) continue;
            particle.x += particle.vx * dt;
            particle.y += particle.vy * dt;
            particle.vx *= 0.94f;
            particle.vy *= 0.94f;
        }

        attackParticles.erase(
            std::remove_if(attackParticles.begin(), attackParticles.end(), [](const AttackParticle& particle) {
                return particle.life <= 0.0f;
            }),
            attackParticles.end());
    }

    void DrawAttackParticles() const {
        for (const AttackParticle& particle : attackParticles) {
            if (particle.life <= 0.0f || particle.maxLife <= 0.0f) continue;
            float alpha = std::clamp(particle.life / particle.maxLife, 0.0f, 1.0f);
            Color tint = Fade(particle.color, alpha);

            int px = static_cast<int>(std::round(particle.x));
            int py = static_cast<int>(std::round(particle.y));
            int size = std::max(1, static_cast<int>(std::round(particle.size)));
            DrawRectangle(px, py, size, size, tint);
        }
    }

    void EmitMovementPuffAtTile(int tileX, int tileY, int count = 6) {
        if (count <= 0) return;
        const float centerX = static_cast<float>(tileX * kTileSize) + static_cast<float>(kTileSize) * 0.5f;
        const float centerY = static_cast<float>(tileY * kTileSize) + static_cast<float>(kTileSize) * 0.72f;

        std::uniform_real_distribution<float> angleDist(0.0f, 6.2831853f);
        std::uniform_real_distribution<float> speedDist(22.0f, 58.0f);
        std::uniform_real_distribution<float> lifeDist(0.22f, 0.40f);
        std::uniform_real_distribution<float> jitterDist(-5.0f, 5.0f);
        std::uniform_real_distribution<float> sizeDist(2.0f, 4.0f);

        Color cloudColor = Color{228, 235, 242, 255};

        for (int i = 0; i < count; ++i) {
            float angle = angleDist(rng);
            float speed = speedDist(rng);
            float life = lifeDist(rng);

            AttackParticle particle;
            particle.x = centerX + jitterDist(rng);
            particle.y = centerY + jitterDist(rng);
            particle.vx = std::cos(angle) * speed;
            particle.vy = std::sin(angle) * speed - 14.0f;
            particle.life = life;
            particle.maxLife = life;
            particle.size = sizeDist(rng);
            particle.color = cloudColor;
            attackParticles.push_back(particle);
        }

        if (attackParticles.size() > static_cast<size_t>(kMaxAttackParticles)) {
            size_t overflow = attackParticles.size() - static_cast<size_t>(kMaxAttackParticles);
            attackParticles.erase(attackParticles.begin(), attackParticles.begin() + static_cast<std::ptrdiff_t>(overflow));
        }
    }

    void EmitProjectileTrailToTile(
        int fromTileX,
        int fromTileY,
        int toTileX,
        int toTileY,
        Color trailColor,
        int particlesPerSegment = 3,
        bool emitDestinationSpark = true,
        float speedMin = 78.0f,
        float speedMax = 148.0f,
        float lifeMin = 0.10f,
        float lifeMax = 0.22f,
        float sizeMin = 1.0f,
        float sizeMax = 2.4f
    ) {
        const float startX = static_cast<float>(fromTileX * kTileSize) + static_cast<float>(kTileSize) * 0.5f;
        const float startY = static_cast<float>(fromTileY * kTileSize) + static_cast<float>(kTileSize) * 0.5f;
        const float endX = static_cast<float>(toTileX * kTileSize) + static_cast<float>(kTileSize) * 0.5f;
        const float endY = static_cast<float>(toTileY * kTileSize) + static_cast<float>(kTileSize) * 0.5f;

        float dx = endX - startX;
        float dy = endY - startY;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len <= 1.0f) {
            if (emitDestinationSpark) {
                EmitAttackParticlesAtTile(toTileX, toTileY, trailColor, 4);
            }
            return;
        }

        float dirX = dx / len;
        float dirY = dy / len;
        int segments = std::max(2, static_cast<int>(std::round(len / (static_cast<float>(kTileSize) * 0.42f))));

        std::uniform_real_distribution<float> jitterDist(-2.0f, 2.0f);
        std::uniform_real_distribution<float> driftDist(-20.0f, 20.0f);
        std::uniform_real_distribution<float> speedDist(std::max(1.0f, speedMin), std::max(std::max(1.0f, speedMin), speedMax));
        std::uniform_real_distribution<float> lifeDist(std::max(0.02f, lifeMin), std::max(std::max(0.02f, lifeMin), lifeMax));
        std::uniform_real_distribution<float> sizeDist(std::max(1.0f, sizeMin), std::max(std::max(1.0f, sizeMin), sizeMax));

        for (int s = 1; s <= segments; ++s) {
            float t = static_cast<float>(s) / static_cast<float>(segments + 1);
            float px = startX + dx * t;
            float py = startY + dy * t;

            for (int p = 0; p < particlesPerSegment; ++p) {
                float speed = speedDist(rng);
                AttackParticle particle;
                particle.x = px + jitterDist(rng);
                particle.y = py + jitterDist(rng);
                particle.vx = dirX * speed + driftDist(rng);
                particle.vy = dirY * speed + driftDist(rng);
                particle.life = lifeDist(rng);
                particle.maxLife = particle.life;
                particle.size = sizeDist(rng);
                particle.color = trailColor;
                attackParticles.push_back(particle);
            }
        }

        if (emitDestinationSpark) {
            EmitAttackParticlesAtTile(toTileX, toTileY, trailColor, 5);
        }

        if (attackParticles.size() > static_cast<size_t>(kMaxAttackParticles)) {
            size_t overflow = attackParticles.size() - static_cast<size_t>(kMaxAttackParticles);
            attackParticles.erase(attackParticles.begin(), attackParticles.begin() + static_cast<std::ptrdiff_t>(overflow));
        }
    }

    void EmitProjectileTrailAlongTiles(const std::vector<std::pair<int, int>>& pathTiles, Color trailColor, int particlesPerSegment = 2) {
        int fromX = player.x;
        int fromY = player.y;
        size_t limit = std::min<size_t>(pathTiles.size(), 28);
        for (size_t i = 0; i < limit; ++i) {
            int toX = pathTiles[i].first;
            int toY = pathTiles[i].second;
            EmitProjectileTrailToTile(fromX, fromY, toX, toY, trailColor, particlesPerSegment, false);
            fromX = toX;
            fromY = toY;
        }
    }

    void EmitProjectileTrailAlongTilesFrom(int fromX, int fromY, const std::vector<std::pair<int, int>>& pathTiles, Color trailColor, int particlesPerSegment = 2) {
        size_t limit = std::min<size_t>(pathTiles.size(), 28);
        for (size_t i = 0; i < limit; ++i) {
            int toX = pathTiles[i].first;
            int toY = pathTiles[i].second;
            EmitProjectileTrailToTile(fromX, fromY, toX, toY, trailColor, particlesPerSegment, false);
            fromX = toX;
            fromY = toY;
        }
    }

    void EmitActiveSkillParticles(ActiveSkillId skillId, int slotIndex, const std::vector<std::pair<int, int>>& resolvedTargets = {}) {
        int branchIndex = ActiveSkillPassiveIndex(skillId);
        Color coreColor = Color{255, 170, 95, 255};
        Color impactColor = Color{255, 220, 140, 255};
        int castCount = 12;
        int tileCountLimit = 24;
        int tileParticleCount = 6;

        if (player.playerClass == PlayerClass::Warrior) {
            if (branchIndex == 0) {
                coreColor = Color{255, 110, 90, 255};
                impactColor = Color{255, 190, 120, 255};
            } else if (branchIndex == 1) {
                coreColor = Color{255, 160, 105, 255};
                impactColor = Color{255, 220, 150, 255};
            } else {
                coreColor = Color{145, 195, 255, 255};
                impactColor = Color{205, 230, 255, 255};
            }
        } else if (player.playerClass == PlayerClass::Ranger) {
            if (branchIndex == 0) {
                coreColor = Color{135, 230, 120, 255};
                impactColor = Color{215, 255, 170, 255};
            } else if (branchIndex == 1) {
                coreColor = Color{120, 230, 195, 255};
                impactColor = Color{190, 250, 230, 255};
            } else {
                coreColor = Color{180, 220, 160, 255};
                impactColor = Color{230, 250, 205, 255};
            }
        } else {
            if (branchIndex == 0) {
                coreColor = Color{145, 175, 255, 255};
                impactColor = Color{210, 225, 255, 255};
            } else if (branchIndex == 1) {
                coreColor = Color{180, 145, 255, 255};
                impactColor = Color{225, 205, 255, 255};
            } else {
                coreColor = Color{165, 225, 255, 255};
                impactColor = Color{215, 245, 255, 255};
            }
        }

        if (skillId == ActiveSkillId::WizardMeteor) {
            coreColor = Color{255, 120, 80, 255};
            impactColor = Color{255, 210, 120, 255};
            castCount = 18;
            tileCountLimit = 32;
            tileParticleCount = 12;
        } else if (skillId == ActiveSkillId::WizardChainLightning) {
            coreColor = Color{120, 170, 255, 255};
            impactColor = Color{215, 235, 255, 255};
            castCount = 14;
            tileCountLimit = 16;
            tileParticleCount = 7;
        } else if (skillId == ActiveSkillId::RangerSmokeBomb) {
            coreColor = Color{185, 195, 205, 255};
            impactColor = Color{220, 225, 232, 255};
            castCount = 10;
            tileCountLimit = 20;
            tileParticleCount = 10;
        } else if (skillId == ActiveSkillId::WarriorWhirlwind) {
            coreColor = Color{255, 130, 95, 255};
            impactColor = Color{255, 185, 130, 255};
            castCount = 16;
            tileCountLimit = 16;
            tileParticleCount = 8;
        } else if (skillId == ActiveSkillId::WizardBlink) {
            coreColor = Color{190, 150, 255, 255};
            impactColor = Color{230, 210, 255, 255};
            castCount = 20;
            tileCountLimit = 8;
            tileParticleCount = 10;
            EmitMovementPuffAtTile(player.x, player.y, 12);
        }

        std::vector<std::pair<int, int>> previewTiles = GetActiveSkillPreviewTiles(slotIndex);

        EmitAttackParticlesAtTile(player.x, player.y, coreColor, castCount);

        bool isLineBeamSkill = (skillId == ActiveSkillId::RangerPiercingBolt || skillId == ActiveSkillId::WizardArcaneBeam);
        bool isMissileSkill = (skillId == ActiveSkillId::WizardMagicMissile);
        bool isCenterProjectileSkill = (skillId == ActiveSkillId::WizardFireball || skillId == ActiveSkillId::WizardMeteor || skillId == ActiveSkillId::RangerVolley || skillId == ActiveSkillId::RangerTangleShot);
        bool isSingleProjectileSkill = (
            skillId == ActiveSkillId::RangerCripplingShot ||
            skillId == ActiveSkillId::RangerAssassinate ||
            skillId == ActiveSkillId::WizardTelekinesis ||
            skillId == ActiveSkillId::RangerPointBlank);

        if (isLineBeamSkill && !previewTiles.empty()) {
            EmitProjectileTrailAlongTiles(previewTiles, coreColor, skillId == ActiveSkillId::WizardArcaneBeam ? 3 : 2);
        } else if (isMissileSkill) {
            std::vector<std::pair<int, int>> targets;
            if (!resolvedTargets.empty()) {
                targets = resolvedTargets;
            } else {
                targets = previewTiles;
                std::sort(targets.begin(), targets.end(), [&](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                    int da = std::abs(a.first - player.x) + std::abs(a.second - player.y);
                    int db = std::abs(b.first - player.x) + std::abs(b.second - player.y);
                    return da < db;
                });
            }
            size_t bolts = std::min<size_t>(targets.size(), 3);
            for (size_t i = 0; i < bolts; ++i) {
                EmitProjectileTrailToTile(player.x, player.y, targets[i].first, targets[i].second, coreColor, 7, true, 34.0f, 72.0f, 0.16f, 0.32f, 2.8f, 4.8f);
            }
        } else if (skillId == ActiveSkillId::WizardChainLightning && !resolvedTargets.empty()) {
            if (resolvedTargets.size() == 1) {
                EmitAttackParticlesAtTile(resolvedTargets.front().first, resolvedTargets.front().second, impactColor, 10);
            } else {
                for (size_t i = 1; i < resolvedTargets.size(); ++i) {
                    const auto& from = resolvedTargets[i - 1];
                    const auto& to = resolvedTargets[i];
                    float linkT = static_cast<float>(i - 1) / static_cast<float>(std::max<size_t>(1, resolvedTargets.size() - 1));
                    unsigned char g = static_cast<unsigned char>(std::clamp(225.0f - linkT * 60.0f, 130.0f, 255.0f));
                    unsigned char b = static_cast<unsigned char>(std::clamp(255.0f - linkT * 18.0f, 180.0f, 255.0f));
                    unsigned char a = static_cast<unsigned char>(std::clamp(255.0f - linkT * 95.0f, 130.0f, 255.0f));
                    Color linkColor{160, g, b, a};
                    int linkParticles = std::max(3, 7 - static_cast<int>(i));
                    EmitProjectileTrailToTile(from.first, from.second, to.first, to.second, linkColor, linkParticles, true, 58.0f, 112.0f, 0.10f, 0.22f, 1.6f, 3.4f);
                    int endpointSparkCount = (i == 1) ? 14 : std::max(4, 8 - static_cast<int>(i));
                    EmitAttackParticlesAtTile(to.first, to.second, Color{245, 250, 255, 255}, endpointSparkCount);
                }
            }
        } else if (isCenterProjectileSkill) {
            int centerX = player.x;
            int centerY = player.y;
            if (!TryGetMouseWorldTile(centerX, centerY) && !previewTiles.empty()) {
                long long sumX = 0;
                long long sumY = 0;
                for (const auto& tile : previewTiles) {
                    sumX += tile.first;
                    sumY += tile.second;
                }
                centerX = static_cast<int>(sumX / static_cast<long long>(previewTiles.size()));
                centerY = static_cast<int>(sumY / static_cast<long long>(previewTiles.size()));
            }
            EmitProjectileTrailToTile(player.x, player.y, centerX, centerY, coreColor, 4, true);
        } else if (isSingleProjectileSkill && !previewTiles.empty()) {
            EmitProjectileTrailToTile(player.x, player.y, previewTiles.front().first, previewTiles.front().second, coreColor, 3, true);
        }

        if (previewTiles.empty()) {
            EmitAttackParticlesAtTile(player.x, player.y, impactColor, std::max(10, tileParticleCount));
            return;
        }

        size_t tileCount = std::min<size_t>(previewTiles.size(), static_cast<size_t>(tileCountLimit));
        for (size_t i = 0; i < tileCount; ++i) {
            int emitCount = tileParticleCount;
            Color emitColor = impactColor;
            if (skillId == ActiveSkillId::WizardChainLightning && (i % 2 == 1)) {
                emitColor = Color{160, 205, 255, 255};
                emitCount += 2;
            }
            EmitAttackParticlesAtTile(previewTiles[i].first, previewTiles[i].second, emitColor, emitCount);

            if (skillId == ActiveSkillId::RangerSmokeBomb) {
                EmitMovementPuffAtTile(previewTiles[i].first, previewTiles[i].second, 4);
            }
        }
    }

    void StartPlayerAttackBumpToward(int targetX, int targetY) {
        float dx = static_cast<float>(targetX - player.x);
        float dy = static_cast<float>(targetY - player.y);
        float len = std::sqrt(dx * dx + dy * dy);
        if (len <= 0.0001f) return;
        player.attackBumpDx = dx / len;
        player.attackBumpDy = dy / len;
        player.attackBumpTimer = kAttackBumpDuration;
    }

    void StartEnemyAttackBumpToward(Enemy& enemy, int targetX, int targetY) {
        float dx = static_cast<float>(targetX - enemy.x);
        float dy = static_cast<float>(targetY - enemy.y);
        float len = std::sqrt(dx * dx + dy * dy);
        if (len <= 0.0001f) return;
        enemy.attackBumpDx = dx / len;
        enemy.attackBumpDy = dy / len;
        enemy.attackBumpTimer = kAttackBumpDuration;
    }

    Vector2 AttackBumpOffset(float dirX, float dirY, float timer) const {
        if (timer <= 0.0f) return Vector2{0.0f, 0.0f};
        float t = std::clamp(timer / kAttackBumpDuration, 0.0f, 1.0f);
        float progress = 1.0f - t;

        float amount = 0.0f;
        if (progress < 0.18f) {
            float p = progress / 0.18f;
            amount = -std::sin(p * 3.1415927f * 0.5f) * (kAttackBumpDistancePx * 0.28f);
        } else {
            float p = (progress - 0.18f) / 0.82f;
            float thrust = std::sin(p * 3.1415927f);
            amount = thrust * (kAttackBumpDistancePx * 1.4f);
        }

        return Vector2{dirX * amount, dirY * amount};
    }

    float EntityPhaseOffset(int tileX, int tileY) const {
        int hash = tileX * 92821 + tileY * 68917;
        hash ^= (hash << 13);
        hash ^= (hash >> 17);
        hash ^= (hash << 5);
        int masked = hash & 1023;
        return static_cast<float>(masked) / 1023.0f * 6.2831853f;
    }

    float EntityRockAngle(int tileX, int tileY, float timeSeconds) const {
        float phase = EntityPhaseOffset(tileX, tileY);
        return std::sin(timeSeconds * kEntityRockSpeed + phase) * kEntityRockAngleDeg;
    }

    float EntityVerticalOffset(int tileX, int tileY, float timeSeconds, bool isFlying) const {
        float phase = EntityPhaseOffset(tileX, tileY);
        if (isFlying) {
            return std::sin(timeSeconds * kFlyingFloatSpeed + phase) * kFlyingFloatPx;
        }
        float cycle = std::sin(timeSeconds * kEntityBounceSpeed + phase);
        return -std::abs(cycle) * kEntityBouncePx;
    }

    float EntityHorizontalOffset(int tileX, int tileY, float timeSeconds, bool isFlying) const {
        if (isFlying) return 0.0f;
        float phase = EntityPhaseOffset(tileX, tileY);
        return std::sin(timeSeconds * kEntityBounceSpeed + phase) * kEntityShufflePx;
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
        if (index == 0) return "Path to your offense active skill (5 nodes)";
        if (index == 1) return "Path to your balanced active skill (5 nodes)";
        return "Path to your defense active skill (5 nodes)";
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

    std::string PassiveStatEffectText(const GeneratedPassive& passive) const {
        int statPct = static_cast<int>(std::round(passive.statPercent * 100.0f));
        std::string sign = passive.increasesStat ? "+" : "-";
        switch (passive.statType) {
        case PassiveStatType::MaxHp: return sign + std::to_string(statPct) + "% Max HP";
        case PassiveStatType::Strength: return sign + std::to_string(statPct) + "% Strength";
        case PassiveStatType::Dexterity: return sign + std::to_string(statPct) + "% Dexterity";
        case PassiveStatType::Intelligence: return sign + std::to_string(statPct) + "% Intelligence";
        case PassiveStatType::Armor: return sign + std::to_string(statPct) + "% Armor";
        case PassiveStatType::Damage: return sign + std::to_string(statPct) + "% Damage dealt";
        case PassiveStatType::GoldDropped: return sign + std::to_string(statPct) + "% Gold dropped";
        case PassiveStatType::ExpEarned: return sign + std::to_string(statPct) + "% EXP gained";
        case PassiveStatType::EnemySpawned: return sign + std::to_string(statPct) + "% Enemies spawned";
        default: return sign + std::to_string(statPct) + "%";
        }
    }

    std::string PassiveSuffixEffectText(const GeneratedPassive& passive) const {
        int suffixPct = static_cast<int>(std::round(passive.passivePercent * 100.0f));
        switch (passive.suffix) {
        case PassiveSuffixId::Vampirism: return "Vampirism: Heal " + std::to_string(suffixPct) + "% of damage dealt";
        case PassiveSuffixId::Larceny: return "Larceny: " + std::to_string(suffixPct) + "% more gold drops";
        case PassiveSuffixId::Punishment: return "Punishment: " + std::to_string(suffixPct) + "% more damage";
        case PassiveSuffixId::Fortress: return "Fortress: +" + std::to_string(suffixPct) + "% Armor";
        case PassiveSuffixId::Echo: return "Echo: " + std::to_string(suffixPct) + "% chance to repeat skills";
        case PassiveSuffixId::Discovery: return "Discovery: " + std::to_string(suffixPct) + "% chance for bonus loot";
        case PassiveSuffixId::Enlightenment: return "Enlightenment: " + std::to_string(suffixPct) + "% more EXP";
        case PassiveSuffixId::Trickery: return "Trickery: " + std::to_string(suffixPct) + "% chance to skip cooldown";
        case PassiveSuffixId::Revival: return "Revival: " + std::to_string(suffixPct) + "% chance to avoid death";
        case PassiveSuffixId::Contagion: return "Contagion: " + std::to_string(suffixPct) + "% death-spread damage";
        case PassiveSuffixId::Conduit: return "Conduit: " + std::to_string(suffixPct) + "% trap conversion";
        case PassiveSuffixId::Charity: return "Charity: " + std::to_string(suffixPct) + "% support buff power";
        case PassiveSuffixId::Miasma: return "Miasma: " + std::to_string(suffixPct) + "% aura damage";
        case PassiveSuffixId::Radiance: return "Radiance: " + std::to_string(suffixPct) + "% holy aura damage";
        default: return std::string(PassiveSuffixName(passive.suffix)) + ": +" + std::to_string(suffixPct) + "%";
        }
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

    int TotalUnlockedTreeSkills() const {
        int total = 0;
        for (int rank : player.classSkillRanks) {
            total += std::clamp(rank, 0, kClassSkillMaxRank);
        }
        return total;
    }

    bool TrySpendSkillPoint(int index) {
        int desiredRank = 1;
        if (index >= 0 && index < kClassSkillCount) {
            desiredRank = std::clamp(player.classSkillRanks[index] + 1, 1, kClassSkillMaxRank);
        }
        return TrySpendSkillPoint(index, desiredRank);
    }

    bool TrySpendSkillPoint(int index, int desiredRank) {
        if (index < 0 || index >= kClassSkillCount) return false;
        if (player.skillPoints <= 0) return false;
        if (TotalUnlockedTreeSkills() >= kSkillTreeSkillCap) return false;

        desiredRank = std::clamp(desiredRank, 1, kClassSkillMaxRank);
        int currentRank = player.classSkillRanks[index];
        if (currentRank >= kClassSkillMaxRank) return false;
        if (desiredRank <= currentRank) return false;
        if (desiredRank != currentRank + 1) return false;

        player.classSkillRanks[index] += 1;
        player.skillPoints -= 1;
        ActiveSkillId unlockedSkill = ActiveSkillId::None;
        if (player.playerClass == PlayerClass::Warrior) unlockedSkill = WarriorBranchSkill(index, player.classSkillRanks[index]);
        else if (player.playerClass == PlayerClass::Ranger) unlockedSkill = RangerBranchSkill(index, player.classSkillRanks[index]);
        else unlockedSkill = WizardBranchSkill(index, player.classSkillRanks[index]);

        RefreshActiveLoadoutFromTree();
        RecomputePlayerStats();
        AddLog(std::string("Upgraded ") + SkillNameForClass(player.playerClass, index) + ".");
        if (unlockedSkill != ActiveSkillId::None) {
            AddLog(std::string("Active unlocked: ") + ActiveSkillName(unlockedSkill) + ".");
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

    ActiveSkillId WarriorBranchSkill(int branchIndex, int rank) const {
        int clampedRank = std::clamp(rank, 1, 5);
        if (branchIndex == 0) {
            static constexpr std::array<ActiveSkillId, 5> offense{
                ActiveSkillId::WarriorCharge,
                ActiveSkillId::WarriorCleave,
                ActiveSkillId::WarriorBash,
                ActiveSkillId::WarriorWhirlwind,
                ActiveSkillId::WarriorDeathblow
            };
            return offense[static_cast<size_t>(clampedRank - 1)];
        }
        if (branchIndex == 1) {
            static constexpr std::array<ActiveSkillId, 5> balanced{
                ActiveSkillId::WarriorPommelStrike,
                ActiveSkillId::WarriorLunge,
                ActiveSkillId::WarriorShout,
                ActiveSkillId::WarriorGrapple,
                ActiveSkillId::WarriorEnvironmentSmash
            };
            return balanced[static_cast<size_t>(clampedRank - 1)];
        }
        static constexpr std::array<ActiveSkillId, 5> defense{
            ActiveSkillId::WarriorHunkerDown,
            ActiveSkillId::WarriorParryRiposte,
            ActiveSkillId::WarriorShieldWall,
            ActiveSkillId::WarriorBodyguard,
            ActiveSkillId::WarriorIndomitable
        };
        return defense[static_cast<size_t>(clampedRank - 1)];
    }

    ActiveSkillId RangerBranchSkill(int branchIndex, int rank) const {
        int clampedRank = std::clamp(rank, 1, 5);
        if (branchIndex == 0) {
            static constexpr std::array<ActiveSkillId, 5> offense{
                ActiveSkillId::RangerSteadyAim,
                ActiveSkillId::RangerPiercingBolt,
                ActiveSkillId::RangerVolley,
                ActiveSkillId::RangerCripplingShot,
                ActiveSkillId::RangerAssassinate
            };
            return offense[static_cast<size_t>(clampedRank - 1)];
        }
        if (branchIndex == 1) {
            static constexpr std::array<ActiveSkillId, 5> balanced{
                ActiveSkillId::RangerPointBlank,
                ActiveSkillId::RangerScoutsEye,
                ActiveSkillId::RangerVault,
                ActiveSkillId::RangerTangleShot,
                ActiveSkillId::RangerFlankingManeuver
            };
            return balanced[static_cast<size_t>(clampedRank - 1)];
        }
        static constexpr std::array<ActiveSkillId, 5> defense{
            ActiveSkillId::RangerSmokeBomb,
            ActiveSkillId::RangerCaltrops,
            ActiveSkillId::RangerCamouflage,
            ActiveSkillId::RangerDecoy,
            ActiveSkillId::RangerCounterTrap
        };
        return defense[static_cast<size_t>(clampedRank - 1)];
    }

    ActiveSkillId WizardBranchSkill(int branchIndex, int rank) const {
        int clampedRank = std::clamp(rank, 1, 5);
        if (branchIndex == 0) {
            static constexpr std::array<ActiveSkillId, 5> offense{
                ActiveSkillId::WizardMagicMissile,
                ActiveSkillId::WizardFireball,
                ActiveSkillId::WizardChainLightning,
                ActiveSkillId::WizardArcaneBeam,
                ActiveSkillId::WizardMeteor
            };
            return offense[static_cast<size_t>(clampedRank - 1)];
        }
        if (branchIndex == 1) {
            static constexpr std::array<ActiveSkillId, 5> balanced{
                ActiveSkillId::WizardManaTap,
                ActiveSkillId::WizardBlink,
                ActiveSkillId::WizardTimeWarp,
                ActiveSkillId::WizardTelekinesis,
                ActiveSkillId::WizardArcaneEcho
            };
            return balanced[static_cast<size_t>(clampedRank - 1)];
        }
        static constexpr std::array<ActiveSkillId, 5> defense{
            ActiveSkillId::WizardFrostNova,
            ActiveSkillId::WizardManaShield,
            ActiveSkillId::WizardMirrorImage,
            ActiveSkillId::WizardRepulsionField,
            ActiveSkillId::WizardStasis
        };
        return defense[static_cast<size_t>(clampedRank - 1)];
    }

    int ActiveSkillRequiredRank(ActiveSkillId skillId) const {
        switch (skillId) {
        case ActiveSkillId::WarriorCharge:
        case ActiveSkillId::WarriorPommelStrike:
        case ActiveSkillId::WarriorHunkerDown:
        case ActiveSkillId::RangerSteadyAim:
        case ActiveSkillId::RangerPointBlank:
        case ActiveSkillId::RangerSmokeBomb:
        case ActiveSkillId::WizardMagicMissile:
        case ActiveSkillId::WizardManaTap:
        case ActiveSkillId::WizardFrostNova:
            return 1;
        case ActiveSkillId::WarriorCleave:
        case ActiveSkillId::WarriorLunge:
        case ActiveSkillId::WarriorParryRiposte:
        case ActiveSkillId::RangerPiercingBolt:
        case ActiveSkillId::RangerScoutsEye:
        case ActiveSkillId::RangerCaltrops:
        case ActiveSkillId::WizardFireball:
        case ActiveSkillId::WizardBlink:
        case ActiveSkillId::WizardManaShield:
            return 2;
        case ActiveSkillId::WarriorBash:
        case ActiveSkillId::WarriorShout:
        case ActiveSkillId::WarriorShieldWall:
        case ActiveSkillId::RangerVolley:
        case ActiveSkillId::RangerVault:
        case ActiveSkillId::RangerCamouflage:
        case ActiveSkillId::WizardChainLightning:
        case ActiveSkillId::WizardTimeWarp:
        case ActiveSkillId::WizardMirrorImage:
            return 3;
        case ActiveSkillId::WarriorWhirlwind:
        case ActiveSkillId::WarriorGrapple:
        case ActiveSkillId::WarriorBodyguard:
        case ActiveSkillId::RangerCripplingShot:
        case ActiveSkillId::RangerTangleShot:
        case ActiveSkillId::RangerDecoy:
        case ActiveSkillId::WizardArcaneBeam:
        case ActiveSkillId::WizardTelekinesis:
        case ActiveSkillId::WizardRepulsionField:
            return 4;
        case ActiveSkillId::WarriorDeathblow:
        case ActiveSkillId::WarriorEnvironmentSmash:
        case ActiveSkillId::WarriorIndomitable:
        case ActiveSkillId::RangerAssassinate:
        case ActiveSkillId::RangerFlankingManeuver:
        case ActiveSkillId::RangerCounterTrap:
        case ActiveSkillId::WizardMeteor:
        case ActiveSkillId::WizardArcaneEcho:
        case ActiveSkillId::WizardStasis:
            return 5;
        default:
            return kClassSkillMaxRank;
        }
    }

    const char* ActiveSkillName(ActiveSkillId skillId) const {
        switch (skillId) {
        case ActiveSkillId::WarriorCharge: return "Charge";
        case ActiveSkillId::WarriorCleave: return "Cleave";
        case ActiveSkillId::WarriorBash: return "Bash";
        case ActiveSkillId::WarriorWhirlwind: return "Whirlwind";
        case ActiveSkillId::WarriorDeathblow: return "Deathblow";
        case ActiveSkillId::WarriorPommelStrike: return "Pommel Strike";
        case ActiveSkillId::WarriorLunge: return "Lunge";
        case ActiveSkillId::WarriorShout: return "Shout";
        case ActiveSkillId::WarriorGrapple: return "Grapple";
        case ActiveSkillId::WarriorEnvironmentSmash: return "Environment Smash";
        case ActiveSkillId::WarriorHunkerDown: return "Hunker Down";
        case ActiveSkillId::WarriorParryRiposte: return "Parry & Riposte";
        case ActiveSkillId::WarriorShieldWall: return "Shield Wall";
        case ActiveSkillId::WarriorBodyguard: return "Bodyguard";
        case ActiveSkillId::WarriorIndomitable: return "Indomitable";
        case ActiveSkillId::RangerSteadyAim: return "Steady Aim";
        case ActiveSkillId::RangerPiercingBolt: return "Piercing Bolt";
        case ActiveSkillId::RangerVolley: return "Volley";
        case ActiveSkillId::RangerCripplingShot: return "Crippling Shot";
        case ActiveSkillId::RangerAssassinate: return "Assassinate";
        case ActiveSkillId::RangerPointBlank: return "Point Blank";
        case ActiveSkillId::RangerScoutsEye: return "Scout's Eye";
        case ActiveSkillId::RangerVault: return "Vault";
        case ActiveSkillId::RangerTangleShot: return "Tangle Shot";
        case ActiveSkillId::RangerFlankingManeuver: return "Flanking Maneuver";
        case ActiveSkillId::RangerSmokeBomb: return "Smoke Bomb";
        case ActiveSkillId::RangerCaltrops: return "Caltrops";
        case ActiveSkillId::RangerCamouflage: return "Camouflage";
        case ActiveSkillId::RangerDecoy: return "Decoy";
        case ActiveSkillId::RangerCounterTrap: return "Counter-Trap";
        case ActiveSkillId::WizardMagicMissile: return "Magic Missile";
        case ActiveSkillId::WizardFireball: return "Fireball";
        case ActiveSkillId::WizardChainLightning: return "Chain Lightning";
        case ActiveSkillId::WizardArcaneBeam: return "Arcane Beam";
        case ActiveSkillId::WizardMeteor: return "Meteor";
        case ActiveSkillId::WizardManaTap: return "Mana Tap";
        case ActiveSkillId::WizardArcaneNova: return "Arcane Nova";
        case ActiveSkillId::WizardBlink: return "Blink";
        case ActiveSkillId::WizardTimeWarp: return "Time Warp";
        case ActiveSkillId::WizardTelekinesis: return "Telekinesis";
        case ActiveSkillId::WizardArcaneEcho: return "Arcane Echo";
        case ActiveSkillId::WizardFrostNova: return "Frost Nova";
        case ActiveSkillId::WizardManaShield: return "Mana Shield";
        case ActiveSkillId::WizardMirrorImage: return "Mirror Image";
        case ActiveSkillId::WizardRepulsionField: return "Repulsion Field";
        case ActiveSkillId::WizardStasis: return "Stasis";
        default: return "Empty";
        }
    }

    const char* ActiveSkillTooltip(ActiveSkillId skillId) const {
        switch (skillId) {
        case ActiveSkillId::WarriorCharge: return "Move in a straight line and attack the first enemy hit.";
        case ActiveSkillId::WarriorCleave: return "Attack a target plus the two adjacent tiles.";
        case ActiveSkillId::WarriorBash: return "Heavy strike: knock back 1 tile and lower enemy defense.";
        case ActiveSkillId::WarriorWhirlwind: return "Spinning attack that strikes all 8 surrounding tiles.";
        case ActiveSkillId::WarriorDeathblow: return "Single-target strike; triple damage on kill, otherwise high miss chance.";
        case ActiveSkillId::WarriorPommelStrike: return "Low damage with a high chance to stun for 1 turn.";
        case ActiveSkillId::WarriorLunge: return "Leap to a visible open tile within 4 range; bypasses traps and obstacles.";
        case ActiveSkillId::WarriorShout: return "Alert enemies and gain a temporary attack buff.";
        case ActiveSkillId::WarriorGrapple: return "Grab an adjacent enemy, disabling it for 2 turns.";
        case ActiveSkillId::WarriorEnvironmentSmash: return "Break wall/furniture and cause area damage.";
        case ActiveSkillId::WarriorHunkerDown: return "End turn, double defense, immune to knockback until next action.";
        case ActiveSkillId::WarriorParryRiposte: return "For 2 turns, counter adjacent enemies that miss you.";
        case ActiveSkillId::WarriorShieldWall: return "Massive defense bonus to yourself (and allies if any).";
        case ActiveSkillId::WarriorBodyguard: return "Protect an adjacent ally for 3 turns; take hits for them.";
        case ActiveSkillId::WarriorIndomitable: return "Heal missing HP and become unstoppable for 5 turns.";
        case ActiveSkillId::RangerSteadyAim: return "Skip movement now; next projectile gains extra range and power.";
        case ActiveSkillId::RangerPiercingBolt: return "Shoot a line that damages every enemy in its path.";
        case ActiveSkillId::RangerVolley: return "Rain arrows on a target 3x3 area.";
        case ActiveSkillId::RangerCripplingShot: return "High-damage arrow that pins the target for 3 turns.";
        case ActiveSkillId::RangerAssassinate: return "Massive strike vs unaware/full-HP targets; ignores part of armor.";
        case ActiveSkillId::RangerPointBlank: return "Adjacent shot that knocks target back 2 tiles.";
        case ActiveSkillId::RangerScoutsEye: return "Reveal nearby enemies and battlefield information.";
        case ActiveSkillId::RangerVault: return "Vault to a visible open tile within 4 range; bypasses traps and obstacles.";
        case ActiveSkillId::RangerTangleShot: return "Seed pod snares enemies in a 3x3 area.";
        case ActiveSkillId::RangerFlankingManeuver: return "For 5 turns, diagonal/back attacks deal double damage.";
        case ActiveSkillId::RangerSmokeBomb: return "Create smoke that blinds nearby enemies.";
        case ActiveSkillId::RangerCaltrops: return "Scatter spikes around you, damaging and hindering foes.";
        case ActiveSkillId::RangerCamouflage: return "Hide near cover until you move.";
        case ActiveSkillId::RangerDecoy: return "Throw a decoy that distracts enemies for several turns.";
        case ActiveSkillId::RangerCounterTrap: return "Set a trap: next enemy in melee is rooted and heavily damaged.";
        case ActiveSkillId::WizardMagicMissile: return "Fire 3 tracking bolts at nearby enemies.";
        case ActiveSkillId::WizardFireball: return "Explodes in a cross pattern and burns targets.";
        case ActiveSkillId::WizardChainLightning: return "Lightning jumps through nearby foes up to 4 times.";
        case ActiveSkillId::WizardArcaneBeam: return "A sustained beam strikes a line for 3 turns.";
        case ActiveSkillId::WizardMeteor: return "Delayed 5x5 blast that shatters walls and enemies.";
        case ActiveSkillId::WizardManaTap: return "Drain an adjacent enemy to restore mana.";
        case ActiveSkillId::WizardArcaneNova: return "Arcane burst centered on you.";
        case ActiveSkillId::WizardBlink: return "Teleport to a visible open tile within 4 range.";
        case ActiveSkillId::WizardTimeWarp: return "Reset last skill cooldown; next spell costs extra mana.";
        case ActiveSkillId::WizardTelekinesis: return "Shove an enemy up to 3 tiles.";
        case ActiveSkillId::WizardArcaneEcho: return "Next 3 turns: spell effects repeat at reduced power.";
        case ActiveSkillId::WizardFrostNova: return "Freeze adjacent enemies for 2 turns.";
        case ActiveSkillId::WizardManaShield: return "For a time, half incoming damage is paid with mana.";
        case ActiveSkillId::WizardMirrorImage: return "Create illusions that can absorb attacks.";
        case ActiveSkillId::WizardRepulsionField: return "Adjacent enemies are pushed away and slowed.";
        case ActiveSkillId::WizardStasis: return "Become invulnerable and regenerate for 3 turns, but cannot act.";
        default: return "No description.";
        }
    }

    const char* ActiveSkillIconTag(ActiveSkillId skillId) const {
        switch (skillId) {
        case ActiveSkillId::WarriorCharge: return "CH";
        case ActiveSkillId::WarriorCleave: return "CL";
        case ActiveSkillId::WarriorBash: return "BA";
        case ActiveSkillId::WarriorWhirlwind: return "WW";
        case ActiveSkillId::WarriorDeathblow: return "DB";
        case ActiveSkillId::WarriorPommelStrike: return "PS";
        case ActiveSkillId::WarriorLunge: return "LU";
        case ActiveSkillId::WarriorShout: return "SH";
        case ActiveSkillId::WarriorGrapple: return "GR";
        case ActiveSkillId::WarriorEnvironmentSmash: return "ES";
        case ActiveSkillId::WarriorHunkerDown: return "HD";
        case ActiveSkillId::WarriorParryRiposte: return "PR";
        case ActiveSkillId::WarriorShieldWall: return "SW";
        case ActiveSkillId::WarriorBodyguard: return "BG";
        case ActiveSkillId::WarriorIndomitable: return "IN";
        case ActiveSkillId::RangerSteadyAim: return "SA";
        case ActiveSkillId::RangerPiercingBolt: return "PB";
        case ActiveSkillId::RangerVolley: return "VO";
        case ActiveSkillId::RangerCripplingShot: return "CS";
        case ActiveSkillId::RangerAssassinate: return "AS";
        case ActiveSkillId::RangerPointBlank: return "PK";
        case ActiveSkillId::RangerScoutsEye: return "SE";
        case ActiveSkillId::RangerVault: return "VA";
        case ActiveSkillId::RangerTangleShot: return "TS";
        case ActiveSkillId::RangerFlankingManeuver: return "FM";
        case ActiveSkillId::RangerSmokeBomb: return "SB";
        case ActiveSkillId::RangerCaltrops: return "CT";
        case ActiveSkillId::RangerCamouflage: return "CM";
        case ActiveSkillId::RangerDecoy: return "DC";
        case ActiveSkillId::RangerCounterTrap: return "TR";
        case ActiveSkillId::WizardMagicMissile: return "MM";
        case ActiveSkillId::WizardFireball: return "FB";
        case ActiveSkillId::WizardChainLightning: return "CL";
        case ActiveSkillId::WizardArcaneBeam: return "AB";
        case ActiveSkillId::WizardMeteor: return "ME";
        case ActiveSkillId::WizardManaTap: return "MT";
        case ActiveSkillId::WizardArcaneNova: return "AN";
        case ActiveSkillId::WizardBlink: return "BL";
        case ActiveSkillId::WizardTimeWarp: return "TW";
        case ActiveSkillId::WizardTelekinesis: return "TK";
        case ActiveSkillId::WizardArcaneEcho: return "AE";
        case ActiveSkillId::WizardFrostNova: return "FN";
        case ActiveSkillId::WizardManaShield: return "MS";
        case ActiveSkillId::WizardMirrorImage: return "MI";
        case ActiveSkillId::WizardRepulsionField: return "RF";
        case ActiveSkillId::WizardStasis: return "ST";
        default: return "--";
        }
    }

    std::string ActiveSkillIconPath(ActiveSkillId skillId) const {
        switch (skillId) {
        case ActiveSkillId::WarriorCharge: return "assets/icons/Active/Warrior/charge.png";
        case ActiveSkillId::WarriorCleave: return "assets/icons/Active/Warrior/cleave.png";
        case ActiveSkillId::WarriorBash: return "assets/icons/Active/Warrior/bash.png";
        case ActiveSkillId::WarriorWhirlwind: return "assets/icons/Active/Warrior/whirlwind.png";
        case ActiveSkillId::WarriorDeathblow: return "assets/icons/Active/Warrior/deathblow.png";
        case ActiveSkillId::WarriorPommelStrike: return "assets/icons/Active/Warrior/pommelStrike.png";
        case ActiveSkillId::WarriorLunge: return "assets/icons/Active/Warrior/lunge.png";
        case ActiveSkillId::WarriorShout: return "assets/icons/Active/Warrior/shout.png";
        case ActiveSkillId::WarriorGrapple: return "assets/icons/Active/Warrior/grapple.png";
        case ActiveSkillId::WarriorEnvironmentSmash: return "assets/icons/Active/Warrior/environmentSmash.png";
        case ActiveSkillId::WarriorHunkerDown: return "assets/icons/Active/Warrior/hunkerDown.png";
        case ActiveSkillId::WarriorParryRiposte: return "assets/icons/Active/Warrior/parryRiposte.png";
        case ActiveSkillId::WarriorShieldWall: return "assets/icons/Active/Warrior/shieldwall.png";
        case ActiveSkillId::WarriorBodyguard: return "assets/icons/Active/Warrior/bodyguard.png";
        case ActiveSkillId::WarriorIndomitable: return "assets/icons/Active/Warrior/indomitable.png";
        case ActiveSkillId::RangerSteadyAim: return "assets/icons/Active/Ranger/steadyAim.png";
        case ActiveSkillId::RangerPiercingBolt: return "assets/icons/Active/Ranger/piercingBolt.png";
        case ActiveSkillId::RangerVolley: return "assets/icons/Active/Ranger/volley.png";
        case ActiveSkillId::RangerCripplingShot: return "assets/icons/Active/Ranger/cripplingShot.png";
        case ActiveSkillId::RangerAssassinate: return "assets/icons/Active/Ranger/assassinate.png";
        case ActiveSkillId::RangerPointBlank: return "assets/icons/Active/Ranger/pointBlank.png";
        case ActiveSkillId::RangerScoutsEye: return "assets/icons/Active/Ranger/scoutsEye.png";
        case ActiveSkillId::RangerVault: return "assets/icons/Active/Ranger/vault.png";
        case ActiveSkillId::RangerTangleShot: return "assets/icons/Active/Ranger/tangleShot.png";
        case ActiveSkillId::RangerFlankingManeuver: return "assets/icons/Active/Ranger/flankingManeuver.png";
        case ActiveSkillId::RangerSmokeBomb: return "assets/icons/Active/Ranger/smokeBomb.png";
        case ActiveSkillId::RangerCaltrops: return "assets/icons/Active/Ranger/caltrops.png";
        case ActiveSkillId::RangerCamouflage: return "assets/icons/Active/Ranger/camouflage.png";
        case ActiveSkillId::RangerDecoy: return "assets/icons/Active/Ranger/decoy.png";
        case ActiveSkillId::RangerCounterTrap: return "assets/icons/Active/Ranger/counterTrap.png";
        case ActiveSkillId::WizardMagicMissile: return "assets/icons/Active/Wizard/magicMissile.png";
        case ActiveSkillId::WizardFireball: return "assets/icons/Active/Wizard/fireBall.png";
        case ActiveSkillId::WizardChainLightning: return "assets/icons/Active/Wizard/chainLightning.png";
        case ActiveSkillId::WizardArcaneBeam: return "assets/icons/Active/Wizard/arcaneBeam.png";
        case ActiveSkillId::WizardMeteor: return "assets/icons/Active/Wizard/meteor.png";
        case ActiveSkillId::WizardManaTap: return "assets/icons/Active/Wizard/manaTap.png";
        case ActiveSkillId::WizardBlink: return "assets/icons/Active/Wizard/blink.png";
        case ActiveSkillId::WizardTimeWarp: return "assets/icons/Active/Wizard/timeWarp.png";
        case ActiveSkillId::WizardTelekinesis: return "assets/icons/Active/Wizard/telekinesis.png";
        case ActiveSkillId::WizardArcaneEcho: return "assets/icons/Active/Wizard/arcaneEcho.png";
        case ActiveSkillId::WizardFrostNova: return "assets/icons/Active/Wizard/frostNova.png";
        case ActiveSkillId::WizardManaShield: return "assets/icons/Active/Wizard/manaShield.png";
        case ActiveSkillId::WizardMirrorImage: return "assets/icons/Active/Wizard/mirrorImage.png";
        case ActiveSkillId::WizardRepulsionField: return "assets/icons/Active/Wizard/repulsionField.png";
        case ActiveSkillId::WizardStasis: return "assets/icons/Active/Wizard/stasis.png";
        default: return "";
        }
    }

    std::string PassiveIconPath(PassiveSuffixId suffixId) const {
        switch (suffixId) {
        case PassiveSuffixId::Vampirism: return "assets/icons/Passive/vampirism.png";
        case PassiveSuffixId::Larceny: return "assets/icons/Passive/larceny.png";
        case PassiveSuffixId::Punishment: return "assets/icons/Passive/punishment.png";
        case PassiveSuffixId::Fortress: return "assets/icons/Passive/fortress.png";
        case PassiveSuffixId::Echo: return "assets/icons/Passive/echo.png";
        case PassiveSuffixId::Discovery: return "assets/icons/Passive/discovery.png";
        case PassiveSuffixId::Enlightenment: return "assets/icons/Passive/enlightenment.png";
        case PassiveSuffixId::Trickery: return "assets/icons/Passive/trickery.png";
        case PassiveSuffixId::Revival: return "assets/icons/Passive/revival.png";
        case PassiveSuffixId::Contagion: return "assets/icons/Passive/contagion.png";
        case PassiveSuffixId::Conduit: return "assets/icons/Passive/conduit.png";
        case PassiveSuffixId::Charity: return "assets/icons/Passive/charity.png";
        case PassiveSuffixId::Miasma: return "assets/icons/Passive/miasma.png";
        case PassiveSuffixId::Radiance: return "assets/icons/Passive/radiance.png";
        default: return "";
        }
    }

    bool TryGetActiveSkillIconTexture(ActiveSkillId skillId, Texture2D& texture) const {
        size_t index = static_cast<size_t>(skillId);
        if (index >= kActiveSkillIconCount) return false;
        if (!activeSkillIconsLoaded[index]) return false;
        texture = activeSkillIcons[index];
        return true;
    }

    bool TryGetPassiveIconTexture(PassiveSuffixId suffixId, Texture2D& texture) const {
        size_t index = static_cast<size_t>(suffixId);
        if (index >= kPassiveIconCount) return false;
        if (!passiveIconsLoaded[index]) return false;
        texture = passiveIcons[index];
        return true;
    }

    int ActiveSkillPassiveIndex(ActiveSkillId skillId) const {
        switch (skillId) {
        case ActiveSkillId::WarriorCharge:
        case ActiveSkillId::WarriorCleave:
        case ActiveSkillId::WarriorBash:
        case ActiveSkillId::WarriorWhirlwind:
        case ActiveSkillId::WarriorDeathblow:
        case ActiveSkillId::RangerSteadyAim:
        case ActiveSkillId::RangerPiercingBolt:
        case ActiveSkillId::RangerVolley:
        case ActiveSkillId::RangerCripplingShot:
        case ActiveSkillId::RangerAssassinate:
        case ActiveSkillId::WizardMagicMissile:
        case ActiveSkillId::WizardFireball:
        case ActiveSkillId::WizardChainLightning:
        case ActiveSkillId::WizardArcaneBeam:
        case ActiveSkillId::WizardMeteor:
        case ActiveSkillId::WizardArcaneNova:
            return 0;
        case ActiveSkillId::WarriorPommelStrike:
        case ActiveSkillId::WarriorLunge:
        case ActiveSkillId::WarriorShout:
        case ActiveSkillId::WarriorGrapple:
        case ActiveSkillId::WarriorEnvironmentSmash:
        case ActiveSkillId::RangerPointBlank:
        case ActiveSkillId::RangerScoutsEye:
        case ActiveSkillId::RangerVault:
        case ActiveSkillId::RangerTangleShot:
        case ActiveSkillId::RangerFlankingManeuver:
        case ActiveSkillId::WizardManaTap:
        case ActiveSkillId::WizardBlink:
        case ActiveSkillId::WizardTimeWarp:
        case ActiveSkillId::WizardTelekinesis:
        case ActiveSkillId::WizardArcaneEcho:
            return 1;
        case ActiveSkillId::WarriorHunkerDown:
        case ActiveSkillId::WarriorParryRiposte:
        case ActiveSkillId::WarriorShieldWall:
        case ActiveSkillId::WarriorBodyguard:
        case ActiveSkillId::WarriorIndomitable:
        case ActiveSkillId::RangerSmokeBomb:
        case ActiveSkillId::RangerCaltrops:
        case ActiveSkillId::RangerCamouflage:
        case ActiveSkillId::RangerDecoy:
        case ActiveSkillId::RangerCounterTrap:
        case ActiveSkillId::WizardFrostNova:
        case ActiveSkillId::WizardManaShield:
        case ActiveSkillId::WizardMirrorImage:
        case ActiveSkillId::WizardRepulsionField:
        case ActiveSkillId::WizardStasis:
            return 2;
        default:
            return -1;
        }
    }

    void RefreshActiveLoadoutFromTree() {
        const auto oldLoadout = player.activeSkillLoadout;
        const auto oldCooldowns = player.activeSkillCooldowns;

        for (size_t i = 0; i < player.activeSkillLoadout.size(); ++i) {
            player.activeSkillLoadout[i] = ActiveSkillId::None;
            player.activeSkillCooldowns[i] = 0;
        }

        auto branchSkillForRank = [&](int branchIndex, int rank)->ActiveSkillId {
            if (player.playerClass == PlayerClass::Warrior) return WarriorBranchSkill(branchIndex, rank);
            if (player.playerClass == PlayerClass::Ranger) return RangerBranchSkill(branchIndex, rank);
            return WizardBranchSkill(branchIndex, rank);
        };

        size_t slot = 0;
        for (int branch = 0; branch < kClassSkillCount && slot < player.activeSkillLoadout.size(); ++branch) {
            int unlockedRanks = std::clamp(player.classSkillRanks[branch], 0, kClassSkillMaxRank);
            for (int rank = 1; rank <= unlockedRanks && slot < player.activeSkillLoadout.size(); ++rank) {
                player.activeSkillLoadout[slot] = branchSkillForRank(branch, rank);
                slot += 1;
            }
        }

        for (size_t newIndex = 0; newIndex < player.activeSkillLoadout.size(); ++newIndex) {
            ActiveSkillId skill = player.activeSkillLoadout[newIndex];
            if (skill == ActiveSkillId::None) continue;
            for (size_t oldIndex = 0; oldIndex < oldLoadout.size(); ++oldIndex) {
                if (oldLoadout[oldIndex] != skill) continue;
                player.activeSkillCooldowns[newIndex] = oldCooldowns[oldIndex];
                break;
            }
        }
    }

    int ActiveSkillBaseCooldown(ActiveSkillId skillId) const {
        switch (skillId) {
        case ActiveSkillId::WarriorCharge: return 3;
        case ActiveSkillId::WarriorCleave: return 4;
        case ActiveSkillId::WarriorBash: return 5;
        case ActiveSkillId::WarriorWhirlwind: return 6;
        case ActiveSkillId::WarriorDeathblow: return 7;
        case ActiveSkillId::WarriorPommelStrike: return 3;
        case ActiveSkillId::WarriorLunge: return 4;
        case ActiveSkillId::WarriorShout: return 5;
        case ActiveSkillId::WarriorGrapple: return 6;
        case ActiveSkillId::WarriorEnvironmentSmash: return 7;
        case ActiveSkillId::WarriorHunkerDown: return 4;
        case ActiveSkillId::WarriorParryRiposte: return 5;
        case ActiveSkillId::WarriorShieldWall: return 6;
        case ActiveSkillId::WarriorBodyguard: return 6;
        case ActiveSkillId::WarriorIndomitable: return 8;
        case ActiveSkillId::RangerSteadyAim: return 2;
        case ActiveSkillId::RangerPiercingBolt: return 4;
        case ActiveSkillId::RangerVolley: return 5;
        case ActiveSkillId::RangerCripplingShot: return 6;
        case ActiveSkillId::RangerAssassinate: return 8;
        case ActiveSkillId::RangerPointBlank: return 3;
        case ActiveSkillId::RangerScoutsEye: return 4;
        case ActiveSkillId::RangerVault: return 5;
        case ActiveSkillId::RangerTangleShot: return 6;
        case ActiveSkillId::RangerFlankingManeuver: return 7;
        case ActiveSkillId::RangerSmokeBomb: return 4;
        case ActiveSkillId::RangerCaltrops: return 5;
        case ActiveSkillId::RangerCamouflage: return 6;
        case ActiveSkillId::RangerDecoy: return 6;
        case ActiveSkillId::RangerCounterTrap: return 7;
        case ActiveSkillId::WizardMagicMissile: return 3;
        case ActiveSkillId::WizardFireball: return 5;
        case ActiveSkillId::WizardChainLightning: return 6;
        case ActiveSkillId::WizardArcaneBeam: return 6;
        case ActiveSkillId::WizardMeteor: return 8;
        case ActiveSkillId::WizardManaTap: return 3;
        case ActiveSkillId::WizardArcaneNova: return 5;
        case ActiveSkillId::WizardBlink: return 4;
        case ActiveSkillId::WizardTimeWarp: return 6;
        case ActiveSkillId::WizardTelekinesis: return 5;
        case ActiveSkillId::WizardArcaneEcho: return 7;
        case ActiveSkillId::WizardFrostNova: return 4;
        case ActiveSkillId::WizardManaShield: return 5;
        case ActiveSkillId::WizardMirrorImage: return 6;
        case ActiveSkillId::WizardRepulsionField: return 6;
        case ActiveSkillId::WizardStasis: return 8;
        default: return 0;
        }
    }

    int GetPressedActiveSkillSlot(const engine::InputSnapshot& input) const {
        for (int i = 0; i < static_cast<int>(input.skillSlotPressed.size()); ++i) {
            if (input.skillSlotPressed[static_cast<size_t>(i)]) return i;
        }
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

        if (player.parryRiposteTurns > 0) {
            player.parryRiposteTurns -= 1;
            if (player.parryRiposteTurns == 0) {
                AddLog("Parry & Riposte fades.");
            }
        }

        if (player.unstoppableTurns > 0) {
            player.unstoppableTurns -= 1;
            if (player.unstoppableTurns == 0) {
                AddLog("Unstoppable fades.");
            }
        }

        if (player.hunkerDownReady && player.temporaryArmorTurns <= 0) {
            player.hunkerDownReady = false;
            player.knockbackImmune = false;
        }

        if (player.rangerFlankingTurns > 0) {
            player.rangerFlankingTurns -= 1;
            if (player.rangerFlankingTurns == 0) {
                AddLog("Flanking Maneuver fades.");
            }
        }

        if (player.rangerCamouflageTurns > 0) {
            player.rangerCamouflageTurns -= 1;
            if (player.rangerCamouflageTurns == 0) {
                AddLog("Camouflage fades.");
            }
        }

        if (player.rangerCounterTrapTurns > 0) {
            player.rangerCounterTrapTurns -= 1;
            if (player.rangerCounterTrapTurns == 0) {
                player.rangerCounterTrapArmed = false;
                AddLog("Counter-Trap expires.");
            }
        }

        if (player.wizardArcaneEchoTurns > 0) {
            player.wizardArcaneEchoTurns -= 1;
            if (player.wizardArcaneEchoTurns == 0) {
                AddLog("Arcane Echo fades.");
            }
        }

        if (player.wizardManaShieldTurns > 0) {
            player.wizardManaShieldTurns -= 1;
            if (player.wizardManaShieldTurns == 0) {
                AddLog("Mana Shield fades.");
            }
        }

        if (player.wizardRepulsionTurns > 0) {
            player.wizardRepulsionTurns -= 1;
            if (player.wizardRepulsionTurns == 0) {
                AddLog("Repulsion Field fades.");
            }
        }

        if (player.wizardStasisTurns > 0) {
            player.wizardStasisTurns -= 1;
            player.mana = std::min(PlayerMaxMana(), player.mana + 1);
            if (player.wizardStasisTurns == 0) {
                AddLog("Stasis ends.");
            }
        }

        if (player.wizardArcaneBeamTurns > 0 && (player.wizardArcaneBeamDx != 0 || player.wizardArcaneBeamDy != 0)) {
            int hits = 0;
            int x = player.x + player.wizardArcaneBeamDx;
            int y = player.y + player.wizardArcaneBeamDy;
            while (InBounds(x, y) && !IsTileOpaque(x, y)) {
                int enemyIndex = -1;
                if (IsOccupiedByEnemy(x, y, &enemyIndex)) {
                    DamageEnemyByPlayer(enemyIndex, std::max(1, RollDamage(player.atk / 2 + 1)));
                    hits += 1;
                }
                x += player.wizardArcaneBeamDx;
                y += player.wizardArcaneBeamDy;
            }
            if (hits > 0) {
                AddLog("Arcane Beam scorches " + std::to_string(hits) + " target(s).");
            }
            player.wizardArcaneBeamTurns -= 1;
            if (player.wizardArcaneBeamTurns == 0) {
                player.wizardArcaneBeamDx = 0;
                player.wizardArcaneBeamDy = 0;
                AddLog("Arcane Beam ends.");
            }
        }

        for (Enemy& enemy : enemies) {
            if (!enemy.alive) continue;
            if (enemy.stunnedTurns > 0) enemy.stunnedTurns -= 1;
            if (enemy.grappledTurns > 0) enemy.grappledTurns -= 1;
            if (enemy.armorBreakTurns > 0) {
                enemy.armorBreakTurns -= 1;
                if (enemy.armorBreakTurns <= 0) {
                    enemy.armorBreakTurns = 0;
                    enemy.armorBreakAmount = 0;
                }
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
        int requiredRank = ActiveSkillRequiredRank(skillId);
        if (requiredPassiveIndex >= 0 && player.classSkillRanks[requiredPassiveIndex] < requiredRank) {
            AddLog(std::string("Need ") + SkillNameForClass(player.playerClass, requiredPassiveIndex) + " rank " + std::to_string(requiredRank) + ".");
            return false;
        }

        if (player.activeSkillCooldowns[slotIndex] > 0) {
            AddLog(std::string(ActiveSkillName(skillId)) + " cooldown: " + std::to_string(player.activeSkillCooldowns[slotIndex]) + " turn(s).");
            return false;
        }

        bool used = false;
        std::vector<std::pair<int, int>> resolvedProjectileTargets;

        auto consumeWizardMana = [&](int baseCost)->bool {
            if (player.playerClass != PlayerClass::Wizard) return true;
            int cost = std::max(0, baseCost);
            if (player.wizardNextSpellManaTax) {
                cost += 1;
            }
            if (player.mana < cost) {
                this->AddLog("Not enough Mana. Need " + std::to_string(cost) + ".");
                return false;
            }
            player.mana -= cost;
            if (player.wizardNextSpellManaTax) {
                player.wizardNextSpellManaTax = false;
                this->AddLog("Time Warp surcharge consumed.");
            }
            return true;
        };

        if (skillId == ActiveSkillId::WarriorCharge) {
            int targetX = 0;
            int targetY = 0;
            if (!TryGetMouseWorldTile(targetX, targetY)) {
                AddLog("No valid direction for Charge.");
                return false;
            }

            auto dir = engine::DominantAxisStepToward(player.x, player.y, targetX, targetY);
            if (dir.first == 0 && dir.second == 0) {
                this->AddLog("No valid direction for Charge.");
                return false;
            }

            int furthestX = player.x;
            int furthestY = player.y;
            for (int step = 1; step <= 6; ++step) {
                int nx = player.x + dir.first * step;
                int ny = player.y + dir.second * step;
                if (!InBounds(nx, ny) || !IsWalkable(nx, ny)) break;

                int enemyIndex = -1;
                if (IsOccupiedByEnemy(nx, ny, &enemyIndex)) {
                    DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk + 1));
                    player.x = furthestX;
                    player.y = furthestY;
                    AddLog("Charge hits target.");
                    used = true;
                    break;
                }

                furthestX = nx;
                furthestY = ny;
            }

            if (!used && (furthestX != player.x || furthestY != player.y)) {
                player.x = furthestX;
                player.y = furthestY;
                AddLog("Charge forward.");
                used = true;
            }
        } else if (skillId == ActiveSkillId::WarriorCleave) {
            int targetX = 0;
            int targetY = 0;
            if (TryGetMouseWorldTile(targetX, targetY) && AttackSwordDirection(targetX, targetY)) {
                AddLog("Cleave sweeps the line.");
                used = true;
            } else {
                AddLog("No valid target for Cleave.");
            }
        } else if (skillId == ActiveSkillId::WarriorBash) {
            int targetX = 0;
            int targetY = 0;
            if (!TryGetMouseWorldTile(targetX, targetY)) return false;
            int enemyIndex = -1;
            if (!IsOccupiedByEnemy(targetX, targetY, &enemyIndex)) {
                AddLog("Bash requires an adjacent enemy.");
                return false;
            }
            if (std::abs(targetX - player.x) + std::abs(targetY - player.y) != 1) {
                AddLog("Bash requires an adjacent enemy.");
                return false;
            }

            Enemy& enemy = enemies[enemyIndex];
            int pushX = enemy.x + (enemy.x - player.x);
            int pushY = enemy.y + (enemy.y - player.y);
            DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk + 2));
            if (enemy.alive && IsWalkable(pushX, pushY) && !IsEnemyAt(pushX, pushY)) {
                enemy.x = pushX;
                enemy.y = pushY;
            }
            if (enemy.alive) {
                enemy.armorBreakAmount = std::max(enemy.armorBreakAmount, 2);
                enemy.armorBreakTurns = std::max(enemy.armorBreakTurns, 2);
            }
            AddLog("Bash crushes and disorients the enemy.");
            used = true;
        } else if (skillId == ActiveSkillId::WarriorWhirlwind) {
            int hits = 0;
            for (int enemyIndex = 0; enemyIndex < static_cast<int>(enemies.size()); ++enemyIndex) {
                Enemy& enemy = enemies[enemyIndex];
                if (!enemy.alive) continue;
                int dx = std::abs(enemy.x - player.x);
                int dy = std::abs(enemy.y - player.y);
                if (dx <= 1 && dy <= 1 && (dx + dy) > 0) {
                    DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk + 3));
                    hits += 1;
                }
            }
            if (hits > 0) {
                AddLog("Whirlwind hits " + std::to_string(hits) + " target(s).");
                used = true;
            } else {
                AddLog("No enemies adjacent for Whirlwind.");
            }
        } else if (skillId == ActiveSkillId::WarriorDeathblow) {
            int targetX = 0;
            int targetY = 0;
            int enemyIndex = -1;
            if (!TryGetMouseWorldTile(targetX, targetY) || !IsOccupiedByEnemy(targetX, targetY, &enemyIndex)) {
                AddLog("Deathblow needs a target.");
                return false;
            }

            Enemy& enemy = enemies[enemyIndex];
            if (RollChance(0.55f)) {
                AddLog("Deathblow misses!");
                used = true;
            } else {
                int baseDamage = RollDamage(player.atk + 4);
                int damage = (enemy.hp <= baseDamage) ? baseDamage * 3 : baseDamage;
                DamageEnemyByPlayer(enemyIndex, damage);
                AddLog("Deathblow lands hard.");
                used = true;
            }
        } else if (skillId == ActiveSkillId::WarriorPommelStrike) {
            int targetX = 0;
            int targetY = 0;
            int enemyIndex = -1;
            if (!TryGetMouseWorldTile(targetX, targetY) || !IsOccupiedByEnemy(targetX, targetY, &enemyIndex) ||
                std::abs(targetX - player.x) + std::abs(targetY - player.y) != 1) {
                AddLog("Pommel Strike needs an adjacent enemy.");
                return false;
            }
            DamageEnemyByPlayer(enemyIndex, std::max(1, RollDamage(std::max(1, player.atk / 2))));
            if (enemies[enemyIndex].alive && RollChance(0.75f)) {
                enemies[enemyIndex].stunnedTurns = std::max(enemies[enemyIndex].stunnedTurns, 1);
                AddLog("Pommel Strike stuns the target.");
            }
            used = true;
        } else if (skillId == ActiveSkillId::WarriorLunge) {
            int targetX = 0;
            int targetY = 0;
            if (TryGetMouseWorldTile(targetX, targetY) && IsWalkable(targetX, targetY) && !IsEnemyAt(targetX, targetY)) {
                int dist = std::abs(targetX - player.x) + std::abs(targetY - player.y);
                if (dist <= 4) {
                    player.x = targetX;
                    player.y = targetY;
                    AddLog("Lunge.");
                    used = true;
                } else {
                    AddLog("Lunge target is too far.");
                }
            } else {
                AddLog("Invalid Lunge target.");
            }
        } else if (skillId == ActiveSkillId::WarriorShout) {
            for (Enemy& enemy : enemies) {
                if (!enemy.alive) continue;
                enemy.seesPlayer = true;
            }
            int buff = std::max(2, player.atk / 4);
            player.temporaryAttackBonus += buff;
            player.temporaryAttackTurns = std::max(player.temporaryAttackTurns, 3);
            RecomputePlayerStats();
            AddLog("Your shout draws attention and boosts attack.");
            used = true;
        } else if (skillId == ActiveSkillId::WarriorGrapple) {
            int targetX = 0;
            int targetY = 0;
            int enemyIndex = -1;
            if (!TryGetMouseWorldTile(targetX, targetY) || !IsOccupiedByEnemy(targetX, targetY, &enemyIndex) ||
                std::abs(targetX - player.x) + std::abs(targetY - player.y) != 1) {
                AddLog("Grapple needs an adjacent enemy.");
                return false;
            }
            enemies[enemyIndex].grappledTurns = std::max(enemies[enemyIndex].grappledTurns, 2);
            DamageEnemyByPlayer(enemyIndex, std::max(1, player.atk / 2));
            AddLog("Enemy grappled for 2 turns.");
            used = true;
        } else if (skillId == ActiveSkillId::WarriorEnvironmentSmash) {
            int targetX = 0;
            int targetY = 0;
            if (!TryGetMouseWorldTile(targetX, targetY)) return false;
            int dist = std::abs(targetX - player.x) + std::abs(targetY - player.y);
            bool smashable = IsTileOpaque(targetX, targetY) || IsAnyDecorationAt(targetX, targetY) || IsAnyInteractableAt(targetX, targetY);
            if (!smashable || dist > 4) {
                AddLog("No environment target in range.");
                return false;
            }
            bool changedTerrain = TryBreakWallTileForEffect(targetX, targetY);
            int hits = 0;
            for (int i = 0; i < static_cast<int>(enemies.size()); ++i) {
                Enemy& enemy = enemies[i];
                if (!enemy.alive) continue;
                int md = std::abs(enemy.x - targetX) + std::abs(enemy.y - targetY);
                if (md <= 1) {
                    DamageEnemyByPlayer(i, RollDamage(player.atk));
                    hits += 1;
                }
            }
            if (changedTerrain) {
                BuildWallsFromOutside();
            }
            AddLog("Environment Smash detonates debris" + std::string(hits > 0 ? "." : " but hits nothing."));
            used = true;
        } else if (skillId == ActiveSkillId::WarriorHunkerDown) {
            player.temporaryArmorBonus += std::max(1, player.armor);
            player.temporaryArmorTurns = std::max(player.temporaryArmorTurns, 1);
            player.hunkerDownReady = true;
            player.knockbackImmune = true;
            RecomputePlayerStats();
            AddLog("Hunker Down: defense doubled until next action.");
            used = true;
        } else if (skillId == ActiveSkillId::WarriorParryRiposte) {
            player.parryRiposteTurns = std::max(player.parryRiposteTurns, 2);
            AddLog("Parry & Riposte is active for 2 turns.");
            used = true;
        } else if (skillId == ActiveSkillId::WarriorShieldWall) {
            player.temporaryArmorBonus += std::max(6, player.armor / 2 + 4);
            player.temporaryArmorTurns = std::max(player.temporaryArmorTurns, 3);
            RecomputePlayerStats();
            AddLog("Shield Wall fortifies your defense.");
            used = true;
        } else if (skillId == ActiveSkillId::WarriorBodyguard) {
            player.temporaryArmorBonus += 5;
            player.temporaryArmorTurns = std::max(player.temporaryArmorTurns, 3);
            RecomputePlayerStats();
            AddLog("Bodyguard stance active (no ally nearby).");
            used = true;
        } else if (skillId == ActiveSkillId::WarriorIndomitable) {
            int missing = std::max(0, player.maxHp - player.hp);
            int heal = std::max(1, static_cast<int>(std::round(static_cast<float>(missing) * 0.45f)));
            player.hp = std::min(player.maxHp, player.hp + heal);
            player.unstoppableTurns = std::max(player.unstoppableTurns, 5);
            player.poisoned = false;
            player.bleeding = false;
            AddLog("Indomitable: healed and unstoppable for 5 turns.");
            used = true;
        } else if (skillId == ActiveSkillId::RangerSteadyAim) {
            player.rangerSteadyAimCharges = 1;
            AddLog("Steady Aim prepared: next projectile gains range and power.");
            used = true;
        } else if (skillId == ActiveSkillId::RangerPiercingBolt) {
            int targetX = 0;
            int targetY = 0;
            if (!TryGetMouseWorldTile(targetX, targetY)) {
                AddLog("No valid direction for Piercing Bolt.");
                return false;
            }

            int relX = targetX - player.x;
            int relY = targetY - player.y;
            if (relX == 0 && relY == 0) {
                AddLog("No valid direction for Piercing Bolt.");
                return false;
            }

            int stepX = (relX > 0) ? 1 : (relX < 0 ? -1 : 0);
            int stepY = (relY > 0) ? 1 : (relY < 0 ? -1 : 0);
            bool boosted = player.rangerSteadyAimCharges > 0;
            if (boosted) {
                player.rangerSteadyAimCharges = 0;
                AddLog("Steady Aim empowers Piercing Bolt.");
            }

            int maxRange = boosted ? 16 : 8;
            int hits = 0;
            for (int step = 1; step <= maxRange; ++step) {
                int nx = player.x + stepX * step;
                int ny = player.y + stepY * step;
                if (!InBounds(nx, ny) || IsTileOpaque(nx, ny)) break;

                int enemyIndex = -1;
                if (IsOccupiedByEnemy(nx, ny, &enemyIndex)) {
                    int bonus = boosted ? std::max(2, player.atk / 2) : 0;
                    DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk + bonus));
                    hits += 1;
                }
            }

            if (hits <= 0) {
                AddLog("Piercing Bolt hits nothing.");
                return false;
            }
            AddLog("Piercing Bolt hits " + std::to_string(hits) + " target(s).");
            used = true;
        } else if (skillId == ActiveSkillId::RangerVolley) {
            int targetX = 0;
            int targetY = 0;
            if (!TryGetMouseWorldTile(targetX, targetY)) {
                AddLog("Volley requires a target area.");
                return false;
            }
            int dist = std::abs(targetX - player.x) + std::abs(targetY - player.y);
            bool boosted = player.rangerSteadyAimCharges > 0;
            int range = boosted ? 12 : 7;
            if (dist > range) {
                AddLog("Volley target is out of range.");
                return false;
            }
            if (boosted) {
                player.rangerSteadyAimCharges = 0;
                AddLog("Steady Aim empowers Volley.");
            }

            int hits = 0;
            for (const auto& offset : engine::SquareAreaOffsets(1)) {
                int tx = targetX + offset.first;
                int ty = targetY + offset.second;
                int enemyIndex = -1;
                if (!InBounds(tx, ty) || !IsOccupiedByEnemy(tx, ty, &enemyIndex)) continue;
                int bonus = boosted ? std::max(1, player.atk / 3) : 0;
                DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk + bonus));
                hits += 1;
            }

            if (hits <= 0) {
                AddLog("Volley finds no enemies in the zone.");
                return false;
            }
            AddLog("Volley strikes " + std::to_string(hits) + " target(s).");
            used = true;
        } else if (skillId == ActiveSkillId::RangerCripplingShot) {
            int targetX = 0;
            int targetY = 0;
            int enemyIndex = -1;
            if (!TryGetMouseWorldTile(targetX, targetY) || !IsOccupiedByEnemy(targetX, targetY, &enemyIndex)) {
                AddLog("Crippling Shot needs an enemy target.");
                return false;
            }
            if (!HasLineOfSight(player.x, player.y, targetX, targetY)) {
                AddLog("No line of sight for Crippling Shot.");
                return false;
            }
            DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk + 3));
            if (enemies[enemyIndex].alive) {
                enemies[enemyIndex].grappledTurns = std::max(enemies[enemyIndex].grappledTurns, 3);
            }
            AddLog("Crippling Shot pins the target for 3 turns.");
            used = true;
        } else if (skillId == ActiveSkillId::RangerAssassinate) {
            int targetX = 0;
            int targetY = 0;
            int enemyIndex = -1;
            if (!TryGetMouseWorldTile(targetX, targetY) || !IsOccupiedByEnemy(targetX, targetY, &enemyIndex)) {
                AddLog("Assassinate needs an enemy target.");
                return false;
            }
            Enemy& enemy = enemies[enemyIndex];
            bool unawareOrFresh = !enemy.seesPlayer || enemy.hp >= enemy.maxHp;
            int armorIgnoreBonus = std::max(0, enemy.atk / 2);
            int damage = RollDamage(player.atk + 4 + armorIgnoreBonus);
            if (unawareOrFresh) {
                damage *= 2;
            }
            DamageEnemyByPlayer(enemyIndex, damage);
            AddLog(unawareOrFresh ? "Assassinate lands a devastating strike." : "Assassinate lands.");
            used = true;
        } else if (skillId == ActiveSkillId::RangerPointBlank) {
            int targetX = 0;
            int targetY = 0;
            int enemyIndex = -1;
            if (!TryGetMouseWorldTile(targetX, targetY) || !IsOccupiedByEnemy(targetX, targetY, &enemyIndex) ||
                std::abs(targetX - player.x) + std::abs(targetY - player.y) != 1) {
                AddLog("Point Blank needs an adjacent enemy.");
                return false;
            }
            Enemy& enemy = enemies[enemyIndex];
            DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk + 2));
            int dirX = enemy.x - player.x;
            int dirY = enemy.y - player.y;
            for (int step = 0; step < 2 && enemy.alive; ++step) {
                int nx = enemy.x + dirX;
                int ny = enemy.y + dirY;
                if (!IsWalkable(nx, ny) || IsEnemyAt(nx, ny, enemyIndex)) break;
                enemy.x = nx;
                enemy.y = ny;
            }
            AddLog("Point Blank knocks the enemy back.");
            used = true;
        } else if (skillId == ActiveSkillId::RangerScoutsEye) {
            int revealed = 0;
            for (Enemy& enemy : enemies) {
                if (!enemy.alive) continue;
                int dist = std::abs(enemy.x - player.x) + std::abs(enemy.y - player.y);
                if (dist <= 10) {
                    enemy.seesPlayer = true;
                    revealed += 1;
                }
            }
            AddLog("Scout's Eye reveals " + std::to_string(revealed) + " nearby enemy position(s).");
            used = true;
        } else if (skillId == ActiveSkillId::RangerVault) {
            int targetX = 0;
            int targetY = 0;
            if (TryGetMouseWorldTile(targetX, targetY) && IsWalkable(targetX, targetY) && !IsEnemyAt(targetX, targetY)) {
                int dist = std::abs(targetX - player.x) + std::abs(targetY - player.y);
                if (dist <= 4) {
                    player.x = targetX;
                    player.y = targetY;
                    AddLog("Vault.");
                    used = true;
                } else {
                    AddLog("Vault target is too far.");
                }
            } else {
                AddLog("Invalid Vault target.");
            }
        } else if (skillId == ActiveSkillId::RangerTangleShot) {
            int targetX = 0;
            int targetY = 0;
            if (!TryGetMouseWorldTile(targetX, targetY)) {
                AddLog("Tangle Shot needs a target area.");
                return false;
            }
            int rooted = 0;
            for (const auto& offset : engine::SquareAreaOffsets(1)) {
                int tx = targetX + offset.first;
                int ty = targetY + offset.second;
                int enemyIndex = -1;
                if (!InBounds(tx, ty) || !IsOccupiedByEnemy(tx, ty, &enemyIndex)) continue;
                enemies[enemyIndex].grappledTurns = std::max(enemies[enemyIndex].grappledTurns, 1);
                rooted += 1;
            }
            AddLog("Tangle Shot snares " + std::to_string(rooted) + " target(s).");
            used = true;
        } else if (skillId == ActiveSkillId::RangerFlankingManeuver) {
            player.rangerFlankingTurns = std::max(player.rangerFlankingTurns, 5);
            AddLog("Flanking Maneuver active for 5 turns.");
            used = true;
        } else if (skillId == ActiveSkillId::RangerSmokeBomb) {
            int blinded = 0;
            for (Enemy& enemy : enemies) {
                if (!enemy.alive) continue;
                int dist = std::abs(enemy.x - player.x) + std::abs(enemy.y - player.y);
                if (dist <= 2) {
                    enemy.seesPlayer = false;
                    enemy.stunnedTurns = std::max(enemy.stunnedTurns, 1);
                    blinded += 1;
                }
            }
            AddLog("Smoke Bomb blinds " + std::to_string(blinded) + " enemy(ies).");
            used = true;
        } else if (skillId == ActiveSkillId::RangerCaltrops) {
            int hits = 0;
            for (int enemyIndex = 0; enemyIndex < static_cast<int>(enemies.size()); ++enemyIndex) {
                Enemy& enemy = enemies[enemyIndex];
                if (!enemy.alive) continue;
                int dist = std::abs(enemy.x - player.x) + std::abs(enemy.y - player.y);
                if (dist <= 1) {
                    DamageEnemyByPlayer(enemyIndex, std::max(1, player.atk / 2));
                    if (enemy.alive) {
                        enemy.stunnedTurns = std::max(enemy.stunnedTurns, 1);
                    }
                    hits += 1;
                }
            }
            AddLog(hits > 0 ? "Caltrops shred nearby enemies." : "Caltrops are scattered around you.");
            used = true;
        } else if (skillId == ActiveSkillId::RangerCamouflage) {
            player.rangerCamouflageTurns = std::max(player.rangerCamouflageTurns, 4);
            for (Enemy& enemy : enemies) {
                if (!enemy.alive) continue;
                enemy.seesPlayer = false;
            }
            AddLog("Camouflage active while you hold position near cover.");
            used = true;
        } else if (skillId == ActiveSkillId::RangerDecoy) {
            for (Enemy& enemy : enemies) {
                if (!enemy.alive) continue;
                enemy.seesPlayer = true;
            }
            AddLog("Decoy deployed: enemies are distracted.");
            used = true;
        } else if (skillId == ActiveSkillId::RangerCounterTrap) {
            player.rangerCounterTrapArmed = true;
            player.rangerCounterTrapTurns = std::max(player.rangerCounterTrapTurns, 5);
            AddLog("Counter-Trap armed.");
            used = true;
        } else if (skillId == ActiveSkillId::WizardMagicMissile) {
            if (!consumeWizardMana(1)) return false;
            int shots = 0;
            for (int pulse = 0; pulse < 3; ++pulse) {
                int bestIndex = -1;
                int bestDist = 999;
                for (int i = 0; i < static_cast<int>(enemies.size()); ++i) {
                    Enemy& enemy = enemies[i];
                    if (!enemy.alive) continue;
                    int dist = std::abs(enemy.x - player.x) + std::abs(enemy.y - player.y);
                    if (dist > 5) continue;
                    if (!HasLineOfSight(player.x, player.y, enemy.x, enemy.y)) continue;
                    if (dist < bestDist) {
                        bestDist = dist;
                        bestIndex = i;
                    }
                }
                if (bestIndex >= 0) {
                    resolvedProjectileTargets.push_back({enemies[bestIndex].x, enemies[bestIndex].y});
                    DamageEnemyByPlayer(bestIndex, std::max(1, RollDamage(player.atk / 2 + 1)));
                    shots += 1;
                }
            }
            if (shots <= 0) {
                AddLog("Magic Missile finds no targets.");
                return false;
            }
            AddLog("Magic Missile fires " + std::to_string(shots) + " bolt(s).");
            used = true;
        } else if (skillId == ActiveSkillId::WizardFireball) {
            if (!consumeWizardMana(2)) return false;
            int targetX = 0;
            int targetY = 0;
            if (!TryGetMouseWorldTile(targetX, targetY)) return false;
            int hits = 0;
            const int shape[5][2] = {{0,0},{1,0},{-1,0},{0,1},{0,-1}};
            for (const auto& offset : shape) {
                int tx = targetX + offset[0];
                int ty = targetY + offset[1];
                int enemyIndex = -1;
                if (!InBounds(tx, ty) || !IsOccupiedByEnemy(tx, ty, &enemyIndex)) continue;
                DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk + 1));
                hits += 1;
            }
            AddLog(hits > 0 ? "Fireball explodes in a cross pattern." : "Fireball bursts harmlessly.");
            used = true;
        } else if (skillId == ActiveSkillId::WizardChainLightning) {
            if (!consumeWizardMana(2)) return false;
            int targetX = 0;
            int targetY = 0;
            int firstIndex = -1;
            if (!TryGetMouseWorldTile(targetX, targetY) || !IsOccupiedByEnemy(targetX, targetY, &firstIndex)) {
                AddLog("Chain Lightning needs an enemy target.");
                return false;
            }
            std::array<int, 4> jumped{-1, -1, -1, -1};
            jumped[0] = firstIndex;
            int jumps = 0;
            int current = firstIndex;
            for (int hop = 0; hop < 4; ++hop) {
                if (current < 0 || !enemies[current].alive) break;
                resolvedProjectileTargets.push_back({enemies[current].x, enemies[current].y});
                DamageEnemyByPlayer(current, RollDamage(player.atk + std::max(0, 2 - hop)));
                jumps += 1;
                int next = -1;
                int best = 999;
                for (int i = 0; i < static_cast<int>(enemies.size()); ++i) {
                    if (!enemies[i].alive || i == current) continue;
                    bool seen = false;
                    for (int v : jumped) if (v == i) { seen = true; break; }
                    if (seen) continue;
                    int dist = std::abs(enemies[i].x - enemies[current].x) + std::abs(enemies[i].y - enemies[current].y);
                    if (dist <= 3 && dist < best) {
                        best = dist;
                        next = i;
                    }
                }
                current = next;
                if (hop + 1 < static_cast<int>(jumped.size())) jumped[hop + 1] = current;
            }
            AddLog("Chain Lightning jumps " + std::to_string(jumps) + " time(s).");
            used = jumps > 0;
        } else if (skillId == ActiveSkillId::WizardArcaneBeam) {
            if (!consumeWizardMana(2)) return false;
            int targetX = 0;
            int targetY = 0;
            if (!TryGetMouseWorldTile(targetX, targetY)) return false;
            int relX = targetX - player.x;
            int relY = targetY - player.y;
            if (relX == 0 && relY == 0) return false;
            int dirX = (relX > 0) ? 1 : (relX < 0 ? -1 : 0);
            int dirY = (relY > 0) ? 1 : (relY < 0 ? -1 : 0);
            if (dirX == 0 && dirY == 0) return false;
            player.wizardArcaneBeamDx = dirX;
            player.wizardArcaneBeamDy = dirY;
            player.wizardArcaneBeamTurns = std::max(player.wizardArcaneBeamTurns, 3);
            AddLog("Arcane Beam begins channeling for 3 turns.");
            used = true;
        } else if (skillId == ActiveSkillId::WizardMeteor) {
            if (!consumeWizardMana(3)) return false;
            int targetX = 0;
            int targetY = 0;
            if (!TryGetMouseWorldTile(targetX, targetY)) return false;
            int hits = 0;
            bool changedTerrain = false;
            for (int y = targetY - 2; y <= targetY + 2; ++y) {
                for (int x = targetX - 2; x <= targetX + 2; ++x) {
                    if (!InBounds(x, y)) continue;
                    int enemyIndex = -1;
                    if (IsOccupiedByEnemy(x, y, &enemyIndex)) {
                        DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk + 6));
                        hits += 1;
                    }
                    changedTerrain = TryBreakWallTileForEffect(x, y) || changedTerrain;
                }
            }
            if (changedTerrain) {
                BuildWallsFromOutside();
            }
            AddLog("Meteor impacts after a brief delay and devastates the area.");
            used = true;
        } else if (skillId == ActiveSkillId::WizardManaTap) {
            int targetX = 0;
            int targetY = 0;
            int enemyIndex = -1;
            if (!TryGetMouseWorldTile(targetX, targetY) || !IsOccupiedByEnemy(targetX, targetY, &enemyIndex) ||
                std::abs(targetX - player.x) + std::abs(targetY - player.y) != 1) {
                AddLog("Mana Tap needs an adjacent enemy.");
                return false;
            }
            DamageEnemyByPlayer(enemyIndex, std::max(1, player.atk / 2));
            int gain = 2;
            player.mana = std::min(PlayerMaxMana(), player.mana + gain);
            AddLog("Mana Tap restores " + std::to_string(gain) + " mana.");
            used = true;
        } else if (skillId == ActiveSkillId::WizardArcaneNova) {
            if (!consumeWizardMana(2)) return false;
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
            if (hits <= 0) {
                AddLog("No enemies in Arcane Nova range.");
                return false;
            }
            AddLog("Arcane Nova hits " + std::to_string(hits) + " target(s).");
            used = true;
        } else if (skillId == ActiveSkillId::WizardBlink) {
            if (!consumeWizardMana(1)) return false;
            int targetX = 0;
            int targetY = 0;
            if (TryGetMouseWorldTile(targetX, targetY) && IsWalkable(targetX, targetY) && !IsEnemyAt(targetX, targetY)) {
                int dist = std::abs(targetX - player.x) + std::abs(targetY - player.y);
                if (dist <= 4) {
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
        } else if (skillId == ActiveSkillId::WizardTimeWarp) {
            if (!consumeWizardMana(1)) return false;
            if (player.wizardLastSkillSlot >= 0 && player.wizardLastSkillSlot < static_cast<int>(player.activeSkillCooldowns.size())) {
                player.activeSkillCooldowns[static_cast<size_t>(player.wizardLastSkillSlot)] = 0;
                AddLog("Time Warp resets your last spell cooldown.");
            } else {
                AddLog("No previous spell to reset.");
            }
            player.wizardNextSpellManaTax = true;
            used = true;
        } else if (skillId == ActiveSkillId::WizardTelekinesis) {
            if (!consumeWizardMana(2)) return false;
            int targetX = 0;
            int targetY = 0;
            int enemyIndex = -1;
            if (!TryGetMouseWorldTile(targetX, targetY) || !IsOccupiedByEnemy(targetX, targetY, &enemyIndex)) {
                AddLog("Telekinesis currently moves enemies only.");
                return false;
            }
            int dx = enemies[enemyIndex].x - player.x;
            int dy = enemies[enemyIndex].y - player.y;
            dx = (dx > 0) ? 1 : (dx < 0 ? -1 : 0);
            dy = (dy > 0) ? 1 : (dy < 0 ? -1 : 0);
            for (int step = 0; step < 3; ++step) {
                int nx = enemies[enemyIndex].x + dx;
                int ny = enemies[enemyIndex].y + dy;
                if (!IsWalkable(nx, ny) || IsEnemyAt(nx, ny, enemyIndex)) break;
                enemies[enemyIndex].x = nx;
                enemies[enemyIndex].y = ny;
            }
            AddLog("Telekinesis hurls the target away.");
            used = true;
        } else if (skillId == ActiveSkillId::WizardArcaneEcho) {
            if (!consumeWizardMana(2)) return false;
            player.wizardArcaneEchoTurns = std::max(player.wizardArcaneEchoTurns, 3);
            AddLog("Arcane Echo primed for 3 turns.");
            used = true;
        } else if (skillId == ActiveSkillId::WizardFrostNova) {
            if (!consumeWizardMana(2)) return false;
            int frozen = 0;
            for (Enemy& enemy : enemies) {
                if (!enemy.alive) continue;
                int md = std::abs(enemy.x - player.x) + std::abs(enemy.y - player.y);
                if (md <= 1 && md > 0) {
                    enemy.stunnedTurns = std::max(enemy.stunnedTurns, 2);
                    frozen += 1;
                }
            }
            AddLog("Frost Nova freezes " + std::to_string(frozen) + " target(s).");
            used = true;
        } else if (skillId == ActiveSkillId::WizardManaShield) {
            if (!consumeWizardMana(1)) return false;
            player.wizardManaShieldTurns = std::max(player.wizardManaShieldTurns, 5);
            AddLog("Mana Shield active.");
            used = true;
        } else if (skillId == ActiveSkillId::WizardMirrorImage) {
            if (!consumeWizardMana(2)) return false;
            player.wizardMirrorImages = std::max(player.wizardMirrorImages, 2);
            AddLog("Mirror Images conjured.");
            used = true;
        } else if (skillId == ActiveSkillId::WizardRepulsionField) {
            if (!consumeWizardMana(2)) return false;
            player.wizardRepulsionTurns = std::max(player.wizardRepulsionTurns, 4);
            AddLog("Repulsion Field surrounds you.");
            used = true;
        } else if (skillId == ActiveSkillId::WizardStasis) {
            if (!consumeWizardMana(3)) return false;
            player.wizardStasisTurns = std::max(player.wizardStasisTurns, 3);
            AddLog("You enter Stasis for 3 turns.");
            used = true;
        }

        if (!used) return false;

        EmitActiveSkillParticles(skillId, slotIndex, resolvedProjectileTargets);

        PassiveRuntimeEffects passiveEffects = ComputePassiveEffects();
        int cooldown = ActiveSkillBaseCooldown(skillId);
        bool skipCooldown = passiveEffects.trickeryChance > 0.0f && RollChance(passiveEffects.trickeryChance);
        if (skipCooldown) {
            player.activeSkillCooldowns[slotIndex] = 0;
            AddLog("Trickery skips cooldown!");
        } else {
            player.activeSkillCooldowns[slotIndex] = std::max(0, cooldown + 1);
        }

        if (player.playerClass == PlayerClass::Wizard && skillId != ActiveSkillId::WizardTimeWarp) {
            player.wizardLastSkillSlot = slotIndex;
        }

        if (allowEcho && player.playerClass == PlayerClass::Wizard && player.wizardArcaneEchoTurns > 0 &&
            skillId != ActiveSkillId::WizardArcaneEcho && skillId != ActiveSkillId::WizardTimeWarp) {
            player.wizardArcaneEchoTurns -= 1;
            AddLog("Arcane Echo repeats the spell!");
            player.activeSkillCooldowns[slotIndex] = 0;
            TryUseActiveSkillSlot(slotIndex, false);
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

    int SolidDecorationVariantCount() const {
        if (!solidsAtlasLoaded) return 0;

        int cols = solidsAtlas.width / kItemIconSize;
        int rows = solidsAtlas.height / kItemIconSize;
        if (cols <= 0 || rows <= 0) return 0;

        if (rows >= 2 && cols == 6) {
            return 11 + std::max(0, rows - 2) * cols;
        }

        return cols * rows;
    }

    bool TryGetSolidDecorationSource(int variant, Rectangle& source) const {
        if (!solidsAtlasLoaded) return false;

        int cols = solidsAtlas.width / kItemIconSize;
        int rows = solidsAtlas.height / kItemIconSize;
        if (cols <= 0 || rows <= 0) return false;

        int tileCount = SolidDecorationVariantCount();
        if (tileCount <= 0) return false;

        int clampedVariant = variant % tileCount;
        if (clampedVariant < 0) clampedVariant = 0;

        int col = 0;
        int row = 0;

        if (rows >= 2 && cols == 6) {
            if (clampedVariant < 5) {
                row = 0;
                col = clampedVariant;
            } else if (clampedVariant < 11) {
                row = 1;
                col = clampedVariant - 5;
            } else {
                int rem = clampedVariant - 11;
                row = 2 + (rem / cols);
                col = rem % cols;
            }
        } else {
            col = clampedVariant % cols;
            row = clampedVariant / cols;
        }

        if (row < 0 || row >= rows || col < 0 || col >= cols) return false;

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
        } else if (tileType == TileType::Wall || tileType == TileType::WallFloor) {
            bool hasFloorBelow = false;
            if (tileY + 1 < kMapHeight) {
                TileType below = tiles[Index(tileX, tileY + 1)];
                hasFloorBelow = (below == TileType::Floor || below == TileType::Stairs);
            }

            if (tileType == TileType::WallFloor) {
                hasFloorBelow = true;
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

    int PlayerMaxMana() const {
        const std::optional<Item>& weaponItem = player.equipped[SlotIndex(EquipSlot::Weapon)];
        int weaponPowerLevel = 3;
        if (weaponItem.has_value()) {
            weaponPowerLevel = std::max(1, static_cast<int>(std::round(weaponItem->itemPower)));
        }
        return std::max(5, 2 + weaponPowerLevel);
    }

    bool IsTileOpaque(int x, int y) const {
        if (!InBounds(x, y)) return true;
        TileType tile = tiles[Index(x, y)];
        return tile == TileType::Wall || tile == TileType::WallFloor || tile == TileType::Outside;
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

        if (player.attackBumpTimer <= 0.0f) {
            StartPlayerAttackBumpToward(enemy.x, enemy.y);
        }

        PassiveRuntimeEffects passiveEffects = ComputePassiveEffects();

        if (player.rangerFlankingTurns > 0) {
            int dx = std::abs(enemy.x - player.x);
            int dy = std::abs(enemy.y - player.y);
            bool diagonal = (dx == 1 && dy == 1);
            bool backAngle = !enemy.seesPlayer;
            if (diagonal || backAngle) {
                damage *= 2;
                AddLog("Flanking Maneuver doubles damage.");
            }
        }

        enemy.hp -= damage;
        WeaponType weaponType = CurrentWeaponType();
        Color attackParticleColor = Color{255, 95, 90, 255};
        int particleCount = 10;
        if (weaponType == WeaponType::Sword) {
            attackParticleColor = Color{255, 110, 95, 255};
            particleCount = 12;
        } else if (weaponType == WeaponType::Bow) {
            attackParticleColor = Color{252, 230, 90, 255};
            particleCount = 8;
        } else {
            attackParticleColor = Color{140, 170, 255, 255};
            particleCount = 14;
        }
        EmitAttackParticlesAtTile(enemy.x, enemy.y, attackParticleColor, particleCount);
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
        auto dir = engine::DominantAxisStepToward(player.x, player.y, targetTileX, targetTileY);
        if (dir.first == 0 && dir.second == 0) return false;

        std::vector<std::pair<int, int>> tilesToHit;
        for (const auto& offset : engine::ForwardArc3Offsets(dir)) {
            tilesToHit.push_back({player.x + offset.first, player.y + offset.second});
        }

        for (const auto& tile : tilesToHit) {
            int enemyIndex = -1;
            if (IsOccupiedByEnemy(tile.first, tile.second, &enemyIndex)) {
                DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk));
            }
        }
        return true;
    }

    bool AttackBowRay(int targetTileX, int targetTileY) {
        auto trailTiles = engine::TraceRayToward(
            player.x,
            player.y,
            targetTileX,
            targetTileY,
            kMapWidth,
            kMapHeight,
            [&](int x, int y) { return IsTileOpaque(x, y); });

        std::vector<std::pair<int, int>> effectiveTrail;
        effectiveTrail.reserve(trailTiles.size());
        for (const auto& tile : trailTiles) {
            effectiveTrail.push_back(tile);
            int enemyIndex = -1;
            if (IsOccupiedByEnemy(tile.first, tile.second, &enemyIndex)) {
                DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk));
                break;
            }
        }

        if (!effectiveTrail.empty()) {
            EmitProjectileTrailAlongTiles(effectiveTrail, Color{252, 228, 120, 255}, 3);
            const auto& endpoint = effectiveTrail.back();
            EmitAttackParticlesAtTile(endpoint.first, endpoint.second, Color{255, 243, 180, 255}, 6);
        }
        return true;
    }

    bool AttackStaffPlus(int targetTileX, int targetTileY) {
        int maxMana = PlayerMaxMana();
        if (player.mana < maxMana) {
            AddLog("Mana is recharging. Move " + std::to_string(maxMana - player.mana) + " more tile(s).");
            return false;
        }

        if (!InBounds(targetTileX, targetTileY) || IsTileOpaque(targetTileX, targetTileY)) {
            return false;
        }
        if (!HasLineOfSight(player.x, player.y, targetTileX, targetTileY)) {
            AddLog("No line of sight for staff cast.");
            return false;
        }

        for (const auto& offset : engine::PlusAreaOffsets(1)) {
            int tx = targetTileX + offset.first;
            int ty = targetTileY + offset.second;
            if (!InBounds(tx, ty)) continue;
            int enemyIndex = -1;
            if (IsOccupiedByEnemy(tx, ty, &enemyIndex)) {
                DamageEnemyByPlayer(enemyIndex, RollDamage(player.atk));
            }
        }

        player.mana = 0;
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
            auto dir = engine::DominantAxisStepToward(player.x, player.y, targetTileX, targetTileY);
            if (dir.first == 0 && dir.second == 0) return false;
            return AttackSwordDirection(dir.first, dir.second);
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
            auto dir = engine::DominantAxisStepToward(player.x, player.y, targetTileX, targetTileY);
            if (dir.first == 0 && dir.second == 0) return preview;
            for (const auto& offset : engine::ForwardArc3Offsets(dir)) {
                preview.push_back({player.x + offset.first, player.y + offset.second});
            }
            return preview;
        }

        if (weaponType == WeaponType::Bow) {
            preview = engine::TraceRayToward(
                player.x,
                player.y,
                targetTileX,
                targetTileY,
                kMapWidth,
                kMapHeight,
                [&](int x, int y) { return IsTileOpaque(x, y); });
            return preview;
        }

        if (InBounds(targetTileX, targetTileY) && HasLineOfSight(player.x, player.y, targetTileX, targetTileY)) {
            for (const auto& offset : engine::PlusAreaOffsets(1)) {
                preview.push_back({targetTileX + offset.first, targetTileY + offset.second});
            }
        }
        return preview;
    }

    std::vector<std::pair<int, int>> GetActiveSkillPreviewTiles(int slotIndex) const {
        std::vector<std::pair<int, int>> preview;
        if (slotIndex < 0 || slotIndex >= static_cast<int>(player.activeSkillLoadout.size())) return preview;

        ActiveSkillId skillId = player.activeSkillLoadout[static_cast<size_t>(slotIndex)];
        if (skillId == ActiveSkillId::None) return preview;

        auto pushTile = [&](int x, int y) {
            if (InBounds(x, y)) preview.push_back({x, y});
        };

        int targetTileX = 0;
        int targetTileY = 0;
        bool hasTarget = TryGetMouseWorldTile(targetTileX, targetTileY);

        switch (skillId) {
        case ActiveSkillId::WarriorWhirlwind:
        case ActiveSkillId::WizardArcaneNova:
        case ActiveSkillId::WizardFrostNova:
            for (const auto& offset : engine::SquareAreaOffsets(1, false)) {
                pushTile(player.x + offset.first, player.y + offset.second);
            }
            return preview;
        case ActiveSkillId::RangerSmokeBomb:
        case ActiveSkillId::RangerCaltrops:
            for (const auto& offset : engine::DiamondAreaOffsets(2)) {
                pushTile(player.x + offset.first, player.y + offset.second);
            }
            return preview;
        case ActiveSkillId::WarriorHunkerDown:
        case ActiveSkillId::WarriorParryRiposte:
        case ActiveSkillId::WarriorShieldWall:
        case ActiveSkillId::WarriorBodyguard:
        case ActiveSkillId::WarriorIndomitable:
        case ActiveSkillId::RangerSteadyAim:
        case ActiveSkillId::RangerScoutsEye:
        case ActiveSkillId::RangerFlankingManeuver:
        case ActiveSkillId::RangerCamouflage:
        case ActiveSkillId::RangerDecoy:
        case ActiveSkillId::RangerCounterTrap:
        case ActiveSkillId::WizardTimeWarp:
        case ActiveSkillId::WizardArcaneEcho:
        case ActiveSkillId::WizardManaShield:
        case ActiveSkillId::WizardMirrorImage:
        case ActiveSkillId::WizardRepulsionField:
        case ActiveSkillId::WizardStasis:
            pushTile(player.x, player.y);
            return preview;
        default:
            break;
        }

        if (!hasTarget) return preview;

        int manhattanDistance = engine::ManhattanDistance(player.x, player.y, targetTileX, targetTileY);
        bool targetHasEnemy = IsEnemyAt(targetTileX, targetTileY);

        switch (skillId) {
        case ActiveSkillId::WarriorBash:
        case ActiveSkillId::WarriorPommelStrike:
        case ActiveSkillId::WarriorGrapple:
        case ActiveSkillId::RangerPointBlank:
        case ActiveSkillId::WizardManaTap:
            if (manhattanDistance != 1 || !targetHasEnemy) return preview;
            break;
        case ActiveSkillId::WarriorDeathblow:
        case ActiveSkillId::RangerAssassinate:
        case ActiveSkillId::WizardChainLightning:
        case ActiveSkillId::WizardTelekinesis:
            if (!targetHasEnemy) return preview;
            break;
        case ActiveSkillId::RangerCripplingShot:
            if (!targetHasEnemy || !HasLineOfSight(player.x, player.y, targetTileX, targetTileY)) return preview;
            break;
        case ActiveSkillId::WarriorEnvironmentSmash: {
            bool smashable = IsTileOpaque(targetTileX, targetTileY) || IsAnyDecorationAt(targetTileX, targetTileY) || IsAnyInteractableAt(targetTileX, targetTileY);
            if (!smashable || manhattanDistance > 4) return preview;
            break;
        }
        case ActiveSkillId::RangerVolley: {
            int range = player.rangerSteadyAimCharges > 0 ? 12 : 7;
            if (manhattanDistance > range) return preview;
            break;
        }
        case ActiveSkillId::WarriorLunge:
        case ActiveSkillId::RangerVault:
        case ActiveSkillId::WizardBlink:
            if (manhattanDistance > 4 || !IsWalkable(targetTileX, targetTileY) || IsEnemyAt(targetTileX, targetTileY)) return preview;
            break;
        default:
            break;
        }

        auto linePreview = [&](int maxSteps = -1) {
            auto tiles = engine::TraceRayToward(
                player.x,
                player.y,
                targetTileX,
                targetTileY,
                kMapWidth,
                kMapHeight,
                [&](int x, int y) { return IsTileOpaque(x, y); },
                maxSteps);
            preview.insert(preview.end(), tiles.begin(), tiles.end());
        };

        switch (skillId) {
        case ActiveSkillId::WarriorCharge:
            linePreview(6);
            break;
        case ActiveSkillId::WizardArcaneBeam:
            linePreview();
            break;
        case ActiveSkillId::RangerPiercingBolt: {
            int maxRange = player.rangerSteadyAimCharges > 0 ? 16 : 8;
            auto tiles = engine::TraceRayToward(
                player.x,
                player.y,
                targetTileX,
                targetTileY,
                kMapWidth,
                kMapHeight,
                [&](int x, int y) { return IsTileOpaque(x, y); },
                maxRange);
            bool hasEnemy = false;
            for (const auto& tile : tiles) {
                preview.push_back(tile);
                if (IsEnemyAt(tile.first, tile.second)) hasEnemy = true;
            }
            if (!hasEnemy) preview.clear();
            break;
        }
        case ActiveSkillId::RangerVolley: {
            bool hasEnemyInZone = false;
            for (const auto& offset : engine::SquareAreaOffsets(1)) {
                int tx = targetTileX + offset.first;
                int ty = targetTileY + offset.second;
                if (InBounds(tx, ty) && IsEnemyAt(tx, ty)) {
                    hasEnemyInZone = true;
                    break;
                }
            }
            if (!hasEnemyInZone) break;
            for (const auto& offset : engine::SquareAreaOffsets(1)) {
                pushTile(targetTileX + offset.first, targetTileY + offset.second);
            }
            break;
        }
        case ActiveSkillId::RangerTangleShot:
            for (const auto& offset : engine::SquareAreaOffsets(1)) {
                pushTile(targetTileX + offset.first, targetTileY + offset.second);
            }
            break;
        case ActiveSkillId::WizardFireball:
            for (const auto& offset : engine::PlusAreaOffsets(1)) {
                pushTile(targetTileX + offset.first, targetTileY + offset.second);
            }
            break;
        case ActiveSkillId::WizardMeteor:
            for (const auto& offset : engine::SquareAreaOffsets(2)) {
                pushTile(targetTileX + offset.first, targetTileY + offset.second);
            }
            break;
        case ActiveSkillId::WarriorCleave: {
            auto dir = engine::DominantAxisStepToward(player.x, player.y, targetTileX, targetTileY);
            if (dir.first == 0 && dir.second == 0) break;
            for (const auto& offset : engine::ForwardArc3Offsets(dir)) {
                pushTile(player.x + offset.first, player.y + offset.second);
            }
            break;
        }
        case ActiveSkillId::WarriorLunge:
        case ActiveSkillId::RangerVault:
            pushTile(targetTileX, targetTileY);
            break;
        case ActiveSkillId::WizardMagicMissile: {
            for (const Enemy& enemy : enemies) {
                if (!enemy.alive) continue;
                int dist = std::abs(enemy.x - player.x) + std::abs(enemy.y - player.y);
                if (dist > 5) continue;
                if (!HasLineOfSight(player.x, player.y, enemy.x, enemy.y)) continue;
                pushTile(enemy.x, enemy.y);
            }
            break;
        }
        default:
            pushTile(targetTileX, targetTileY);
            break;
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

    const char* CharacterMenuTabLabel(CharacterMenuTab tab) const {
        switch (tab) {
        case CharacterMenuTab::Inventory: return "Inventory";
        case CharacterMenuTab::Stats: return "Stats";
        case CharacterMenuTab::SkillTree: return "Skill Tree";
        default: return "Inventory";
        }
    }

    Rectangle CharacterMenuTabRect(int tabIndex) const {
        const int panelH = 620;
        int panelY = (kScreenHeight - panelH) / 2;

        const int tabW = 190;
        const int tabH = 44;
        const int tabGap = 14;
        const int totalW = tabW * 3 + tabGap * 2;
        const int startX = (kScreenWidth - totalW) / 2;
        const int y = std::max(8, panelY - tabH - 8);
        return Rectangle{
            static_cast<float>(startX + tabIndex * (tabW + tabGap)),
            static_cast<float>(y),
            static_cast<float>(tabW),
            static_cast<float>(tabH)
        };
    }

    int EquippedStrengthBonus() const {
        int bonus = 0;
        for (const auto& eq : player.equipped) {
            if (!eq.has_value()) continue;
            bonus += eq->strBonus;
        }
        return bonus;
    }

    int EquippedDexterityBonus() const {
        int bonus = 0;
        for (const auto& eq : player.equipped) {
            if (!eq.has_value()) continue;
            bonus += eq->dexBonus;
        }
        return bonus;
    }

    int EquippedStaminaBonus() const {
        int bonus = 0;
        for (const auto& eq : player.equipped) {
            if (!eq.has_value()) continue;
            bonus += eq->hpBonus;
        }
        return bonus;
    }

    bool TrySpendAttributePoint(int statIndex) {
        if (player.skillPoints <= 0) return false;

        if (statIndex == 0) {
            player.baseStrength += 1;
            player.skillPoints -= 1;
            RecomputePlayerStats();
            AddLog("Base Strength increased by 1.");
            return true;
        }
        if (statIndex == 1) {
            player.baseDexterity += 1;
            player.skillPoints -= 1;
            RecomputePlayerStats();
            AddLog("Base Dexterity increased by 1.");
            return true;
        }
        if (statIndex == 2) {
            player.baseMaxHp += 2;
            player.skillPoints -= 1;
            RecomputePlayerStats();
            AddLog("Base Stamina increased (+2 Max HP).");
            return true;
        }

        return false;
    }

    void HandleStatsMenuInput() {
        statsAttributeSelection = std::clamp(statsAttributeSelection, 0, 2);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            const int panelW = 1020;
            const int panelH = 620;
            const int panelX = (kScreenWidth - panelW) / 2;
            const int panelY = (kScreenHeight - panelH) / 2;
            const int statsY = panelY + 154;
            const int rowH = 54;
            const int rowW = 420;
            const int rowX = panelX + 34;
            for (int i = 0; i < 3; ++i) {
                Rectangle rowRect{static_cast<float>(rowX), static_cast<float>(statsY + i * rowH), static_cast<float>(rowW), static_cast<float>(rowH - 6)};
                if (CheckCollisionPointRec(mousePos, rowRect)) {
                    statsAttributeSelection = i;
                    if (!TrySpendAttributePoint(statsAttributeSelection)) {
                        PlaySfx(sfxDecline, sfxDeclineLoaded);
                        AddLog("No skill points available.");
                    } else {
                        PlaySfx(sfxInteract, sfxInteractLoaded);
                    }
                    return;
                }
            }
        }

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            statsAttributeSelection -= 1;
            if (statsAttributeSelection < 0) statsAttributeSelection = 2;
        } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            statsAttributeSelection += 1;
            if (statsAttributeSelection > 2) statsAttributeSelection = 0;
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE)) {
            if (!TrySpendAttributePoint(statsAttributeSelection)) {
                PlaySfx(sfxDecline, sfxDeclineLoaded);
                AddLog("No skill points available.");
            } else {
                PlaySfx(sfxInteract, sfxInteractLoaded);
            }
        }
    }

    void HandleCharacterMenuInput() {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_TAB)) {
            PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            characterMenuOpen = false;
            return;
        }

        Vector2 mousePos = GetMousePosition();
        bool mouseOverTabStrip = false;
        for (int i = 0; i < 3; ++i) {
            if (CheckCollisionPointRec(mousePos, CharacterMenuTabRect(i))) {
                mouseOverTabStrip = true;
                break;
            }
        }

        if (mouseOverTabStrip && (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))) {
            int tab = static_cast<int>(characterMenuTab) - 1;
            if (tab < 0) tab = 2;
            characterMenuTab = static_cast<CharacterMenuTab>(tab);
            PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
            return;
        }
        if (mouseOverTabStrip && (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))) {
            int tab = static_cast<int>(characterMenuTab) + 1;
            if (tab > 2) tab = 0;
            characterMenuTab = static_cast<CharacterMenuTab>(tab);
            PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
            return;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            for (int i = 0; i < 3; ++i) {
                if (!CheckCollisionPointRec(mousePos, CharacterMenuTabRect(i))) continue;
                CharacterMenuTab nextTab = static_cast<CharacterMenuTab>(i);
                if (nextTab != characterMenuTab) {
                    characterMenuTab = nextTab;
                    PlaySfx(sfxMenuOpen, sfxMenuOpenLoaded);
                }
                return;
            }
        }

        if (characterMenuTab == CharacterMenuTab::Inventory) {
            HandleInventoryInput();
            return;
        }
        if (characterMenuTab == CharacterMenuTab::SkillTree) {
            HandleSkillTreeInput();
            return;
        }
        HandleStatsMenuInput();
    }

    void HandleSkillTreeInput() {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_TAB)) {
            PlaySfx(sfxMenuClose, sfxMenuCloseLoaded);
            characterMenuOpen = false;
            return;
        }

        const int panelW = 1020;
        const int panelH = 620;
        const int panelX = (kScreenWidth - panelW) / 2;
        const int panelY = (kScreenHeight - panelH) / 2;
        const int branchTopY = panelY + 214;
        const int branchBottomY = panelY + 510;
        const int nodeGapY = (branchBottomY - branchTopY) / std::max(1, (kClassSkillMaxRank - 1));
        const int branchSpacing = 300;
        const int leftBranchX = panelX + (panelW - branchSpacing * 2) / 2;

        Vector2 mousePos = GetMousePosition();
        for (int i = 0; i < kClassSkillCount; ++i) {
            int bx = leftBranchX + i * branchSpacing;
            for (int p = 0; p < kClassSkillMaxRank; ++p) {
                int nodeY = branchTopY + p * nodeGapY;
                Rectangle frameRect{static_cast<float>(bx - 16), static_cast<float>(nodeY - 16), 32.0f, 32.0f};
                if (!CheckCollisionPointRec(mousePos, frameRect)) continue;
                skillSelection = i;
                skillRankSelection = p + 1;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (TrySpendSkillPoint(skillSelection, skillRankSelection)) {
                        PlaySfx(sfxInteract, sfxInteractLoaded);
                    } else {
                        PlaySfx(sfxDecline, sfxDeclineLoaded);
                        if (TotalUnlockedTreeSkills() >= kSkillTreeSkillCap) {
                            AddLog("Skill tree locked: 10 skills reached.");
                        } else if (player.skillPoints <= 0) {
                            AddLog("No skill points available.");
                        } else if (player.classSkillRanks[skillSelection] >= kClassSkillMaxRank) {
                            AddLog("Skill is already maxed.");
                        } else if (skillRankSelection > player.classSkillRanks[skillSelection] + 1) {
                            AddLog("Unlock earlier ranks first.");
                        } else {
                            AddLog("That rank is already unlocked.");
                        }
                    }
                }
                break;
            }
        }

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
            skillSelection -= 1;
            if (skillSelection < 0) skillSelection = kClassSkillCount - 1;
        } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
            skillSelection += 1;
            if (skillSelection >= kClassSkillCount) skillSelection = 0;
        }

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            skillRankSelection -= 1;
            if (skillRankSelection < 1) skillRankSelection = kClassSkillMaxRank;
        } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            skillRankSelection += 1;
            if (skillRankSelection > kClassSkillMaxRank) skillRankSelection = 1;
        }

        skillRankSelection = std::clamp(skillRankSelection, 1, kClassSkillMaxRank);

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE)) {
            if (TrySpendSkillPoint(skillSelection, skillRankSelection)) {
                PlaySfx(sfxInteract, sfxInteractLoaded);
            } else {
                PlaySfx(sfxDecline, sfxDeclineLoaded);
                if (TotalUnlockedTreeSkills() >= kSkillTreeSkillCap) {
                    AddLog("Skill tree locked: 10 skills reached.");
                } else if (player.skillPoints <= 0) {
                    AddLog("No skill points available.");
                } else if (player.classSkillRanks[skillSelection] >= kClassSkillMaxRank) {
                    AddLog("Skill is already maxed.");
                } else if (skillRankSelection > player.classSkillRanks[skillSelection] + 1) {
                    AddLog("Unlock earlier ranks first.");
                } else {
                    AddLog("That rank is already unlocked.");
                }
            }
        }
    }

    void HandlePassiveChoiceInput() {
        if (!passiveChoiceOpen) return;

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
        int hoveredCard = -1;
        for (int i = 0; i < 3; ++i) {
            int x = startX + i * (cardW + gap);
            Rectangle cardRect{static_cast<float>(x), static_cast<float>(cardY), static_cast<float>(cardW), static_cast<float>(cardH)};
            if (CheckCollisionPointRec(mousePos, cardRect)) {
                hoveredCard = i;
                break;
            }
        }

        if (hoveredCard >= 0) {
            passiveChoiceSelection = hoveredCard;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                ApplyPassiveChoice(hoveredCard);
                PlaySfx(sfxInteract, sfxInteractLoaded);
                return;
            }
        }

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
    }

    void DrawActiveSkillHotbar() const {
        if (classSelectionPending || characterMenuOpen || merchantOpen || pauseMenuOpen || gameOver || devConsoleOpen) {
            return;
        }

        const int mapW = kMapWidth * kTileSize;
        const int mapH = kMapHeight * kTileSize;
        const int slotCount = 10;
        const int slotSize = 32;
        const int slotGap = 6;
        const int totalW = slotCount * slotSize + (slotCount - 1) * slotGap;
        const int startX = (mapW - totalW) / 2;
        const int y = mapH - slotSize - 8;
        Vector2 mousePos = GetMousePosition();
        ActiveSkillId hoveredSkill = ActiveSkillId::None;
        Rectangle hoveredRect{};

        const char* keyLabels[10] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};

        for (int i = 0; i < slotCount; ++i) {
            int x = startX + i * (slotSize + slotGap);
            Rectangle rect{static_cast<float>(x), static_cast<float>(y), static_cast<float>(slotSize), static_cast<float>(slotSize)};

            ActiveSkillId skillId = player.activeSkillLoadout[static_cast<size_t>(i)];
            int requiredBranch = ActiveSkillPassiveIndex(skillId);
            int requiredRank = ActiveSkillRequiredRank(skillId);
            bool unlocked = (skillId != ActiveSkillId::None) && (requiredBranch < 0 || player.classSkillRanks[requiredBranch] >= requiredRank);
            int cooldown = player.activeSkillCooldowns[static_cast<size_t>(i)];

            Color fill = Color{20, 24, 34, 220};
            if (unlocked && cooldown <= 0) fill = Color{42, 62, 98, 235};
            if (unlocked && cooldown > 0) fill = Color{52, 46, 68, 235};

            DrawRectangleRec(rect, fill);
            DrawRectangleLinesEx(rect, 2.0f, Color{95, 110, 145, 255});
            if (i == player.armedActiveSkillSlot) {
                DrawRectangleLinesEx(Rectangle{rect.x - 2.0f, rect.y - 2.0f, rect.width + 4.0f, rect.height + 4.0f}, 2.0f, Color{150, 220, 255, 255});
            }

            Rectangle iconRect{rect.x + 2.0f, rect.y + 2.0f, 28.0f, 28.0f};
            Color iconFill = unlocked ? Color{85, 110, 168, 255} : Color{58, 52, 58, 255};
            DrawRectangleRec(iconRect, iconFill);
            Texture2D iconTexture{};
            if (TryGetActiveSkillIconTexture(skillId, iconTexture)) {
                Rectangle src{0.0f, 0.0f, static_cast<float>(iconTexture.width), static_cast<float>(iconTexture.height)};
                Rectangle dst{iconRect.x + 2.0f, iconRect.y + 2.0f, iconRect.width - 4.0f, iconRect.height - 4.0f};
                DrawTexturePro(iconTexture, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
            } else {
                const char* iconTag = ActiveSkillIconTag(skillId);
                int iconW = MeasureText(iconTag, 12);
                DrawText(iconTag, static_cast<int>(iconRect.x + (iconRect.width - static_cast<float>(iconW)) * 0.5f), static_cast<int>(iconRect.y + 8.0f), 12, WHITE);
            }

            int keyW = MeasureText(keyLabels[i], 11);
            DrawText(keyLabels[i], x + slotSize - keyW - 3, y + 2, 11, Color{210, 220, 245, 255});

            if (unlocked && cooldown > 0) {
                DrawText(TextFormat("%i", cooldown), x + slotSize - 10, y + 1, 12, Color{255, 220, 130, 255});
            }

            if (CheckCollisionPointRec(mousePos, rect) && skillId != ActiveSkillId::None) {
                hoveredSkill = skillId;
                hoveredRect = rect;
            }
        }

        if (hoveredSkill != ActiveSkillId::None) {
            const char* title = ActiveSkillName(hoveredSkill);
            const char* desc = ActiveSkillTooltip(hoveredSkill);
            const int tipW = 390;
            const int tipH = 86;
            int tipX = static_cast<int>(hoveredRect.x + hoveredRect.width * 0.5f) - tipW / 2;
            int tipY = static_cast<int>(hoveredRect.y) - tipH - 10;
            tipX = std::clamp(tipX, 8, mapW - tipW - 8);
            tipY = std::max(8, tipY);

            DrawRectangle(tipX, tipY, tipW, tipH, Color{12, 14, 22, 238});
            DrawRectangleLinesEx(Rectangle{static_cast<float>(tipX), static_cast<float>(tipY), static_cast<float>(tipW), static_cast<float>(tipH)}, 2.0f, Color{95, 120, 165, 255});
            DrawText(title, tipX + 10, tipY + 8, 22, RAYWHITE);
            DrawText(desc, tipX + 10, tipY + 38, 18, Color{195, 210, 236, 255});
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
            characterMenuOpen = false;
            return;
        }

        const int panelW = 1020;
        const int panelH = 620;
        const int panelX = (kScreenWidth - panelW) / 2;
        const int panelY = (kScreenHeight - panelH) / 2;

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

        player.mana = std::clamp(player.mana, 0, PlayerMaxMana());

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
        return engine::GridIndex(x, y, kMapWidth);
    }

    bool InBounds(int x, int y) const {
        return engine::InBounds(x, y, kMapWidth, kMapHeight);
    }

    bool IsWalkable(int x, int y) const {
        if (!InBounds(x, y)) return false;
        TileType t = tiles[Index(x, y)];
        if (!(t == TileType::Floor || t == TileType::Stairs)) return false;
        if (IsSolidDecorationAt(x, y)) return false;
        if (IsBlockingInteractableAt(x, y)) return false;
        return true;
    }

    bool IsAdjacentToCover(int x, int y) const {
        return engine::HasNeighborMatching(
            x,
            y,
            kMapWidth,
            kMapHeight,
            [&](int nx, int ny) {
                return IsTileOpaque(nx, ny) || IsAnyDecorationAt(nx, ny) || IsAnyInteractableAt(nx, ny);
            },
            true);
    }

    bool HasPathToTile(int goalX, int goalY) const {
        return engine::HasPathBfs(
            kMapWidth,
            kMapHeight,
            player.x,
            player.y,
            goalX,
            goalY,
            [&](int x, int y) {
                return IsWalkable(x, y);
            });
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
        std::vector<TileType> snapshot = tiles;

        auto tileAt = [&](int x, int y) {
            return snapshot[Index(x, y)];
        };

        engine::BuildWallsFromOutside(
            kMapWidth,
            kMapHeight,
            [&](int x, int y) {
                return tileAt(x, y) == TileType::Outside;
            },
            [&](int x, int y) {
                TileType tile = tileAt(x, y);
                return tile == TileType::Floor || tile == TileType::Stairs;
            },
            [&](int x, int y, bool floorBelow) {
                tiles[Index(x, y)] = floorBelow ? TileType::WallFloor : TileType::Wall;
            });
    }

    bool TryBreakWallTileForEffect(int x, int y) {
        if (!InBounds(x, y)) return false;
        TileType& tile = tiles[Index(x, y)];
        if (tile == TileType::Wall || tile == TileType::WallFloor) {
            tile = TileType::Floor;
            return true;
        }
        return false;
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

    bool HasLineOfSight(int x0, int y0, int x1, int y1) const {
        return engine::HasLineOfSight(
            x0,
            y0,
            x1,
            y1,
            kMapWidth,
            kMapHeight,
            [&](int x, int y) {
                TileType blockTile = tiles[Index(x, y)];
                return blockTile == TileType::Wall || blockTile == TileType::WallFloor;
            });
    }

    std::vector<std::pair<int, int>> FindPathAStar(int sx, int sy, int gx, int gy, int selfEnemyIndex) const {
        std::vector<std::pair<int, int>> empty;
        if (!IsWalkableForEnemy(sx, sy, selfEnemyIndex) || !IsWalkable(gx, gy)) return empty;

        auto isWalkable = [&](int x, int y) {
            if (x == gx && y == gy) return IsWalkable(x, y);
            return IsWalkableForEnemy(x, y, selfEnemyIndex);
        };

        return engine::FindPathAStar(
            kMapWidth,
            kMapHeight,
            sx,
            sy,
            gx,
            gy,
            isWalkable);
    }

    void ApplyClassPreset(PlayerClass cls) {
        player.playerClass = cls;
        player.skillPoints = 0;
        player.classSkillRanks = {0, 0, 0};
        player.activeSkillCooldowns = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        player.armedActiveSkillSlot = -1;
        player.temporaryArmorBonus = 0;
        player.temporaryArmorTurns = 0;
        player.temporaryAttackBonus = 0;
        player.temporaryAttackTurns = 0;
        player.parryRiposteTurns = 0;
        player.hunkerDownReady = false;
        player.unstoppableTurns = 0;
        player.knockbackImmune = false;
        player.rangerSteadyAimCharges = 0;
        player.rangerFlankingTurns = 0;
        player.rangerCamouflageTurns = 0;
        player.rangerCounterTrapArmed = false;
        player.rangerCounterTrapTurns = 0;
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
            starterWeapon.name = "Broken Sword";
            starterWeapon.weaponType = WeaponType::Sword;
            break;
        case PlayerClass::Ranger:
            player.baseMaxHp = 22;
            player.baseStrength = 5;
            player.baseDexterity = 10;
            player.baseIntelligence = 3;
            starterWeapon.name = "Worn Bow";
            starterWeapon.weaponType = WeaponType::Bow;
            break;
        case PlayerClass::Wizard:
            player.baseMaxHp = 18;
            player.baseStrength = 3;
            player.baseDexterity = 5;
            player.baseIntelligence = 10;
            starterWeapon.name = "Uncharged Staff";
            starterWeapon.weaponType = WeaponType::Staff;
            break;
        }

        player.hp = player.baseMaxHp;
        player.mana = (cls == PlayerClass::Wizard) ? 0 : 5;
        player.inventory.clear();
        inventoryScrollOffset = 0;
        for (auto& s : player.equipped) s.reset();
        player.equipped[SlotIndex(EquipSlot::Weapon)] = starterWeapon;
        if (cls != PlayerClass::Wizard) {
            player.mana = PlayerMaxMana();
        }

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
    RefreshActiveLoadoutFromTree();
        characterMenuOpen = false;
        characterMenuTab = CharacterMenuTab::Inventory;
        skillSelection = 0;
        skillRankSelection = 1;
        statsAttributeSelection = 0;
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
        enemy.mana = 5;
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
            AddDevConsoleLine("  addlevel [quantity]");
            AddDevConsoleLine("  setlevel [quantity]");
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

        if (command == "addlevel") {
            if (tokens.size() < 2) {
                AddDevConsoleLine("Usage: addlevel [quantity]");
                return;
            }

            int quantity = 0;
            try {
                quantity = std::stoi(tokens[1]);
            } catch (...) {
                AddDevConsoleLine("Quantity must be a number.");
                return;
            }

            if (quantity <= 0) {
                AddDevConsoleLine("Quantity must be greater than 0.");
                return;
            }

            int startLevel = player.level;
            int draftsUnlocked = 0;
            for (int i = 0; i < quantity; ++i) {
                draftsUnlocked += 1;
                ApplySingleLevelUp(true);
            }

            if (!passiveChoiceOpen && pendingPassiveChoices > 0) {
                pendingPassiveChoices -= 1;
                GeneratePassiveChoiceOffer();
            }

            AddDevConsoleLine("Level increased from " + std::to_string(startLevel) + " to " + std::to_string(player.level) + ".");
            AddDevConsoleLine("Passive drafts unlocked: " + std::to_string(draftsUnlocked) + ".");
            AddLog("[DEV] Added " + std::to_string(quantity) + " level(s).");
            return;
        }

        if (command == "setlevel") {
            if (tokens.size() < 2) {
                AddDevConsoleLine("Usage: setlevel [quantity]");
                return;
            }

            int targetLevel = 0;
            try {
                targetLevel = std::stoi(tokens[1]);
            } catch (...) {
                AddDevConsoleLine("Quantity must be a number.");
                return;
            }

            if (targetLevel < 1) {
                AddDevConsoleLine("Quantity must be at least 1.");
                return;
            }

            int previousLevel = player.level;
            player.level = targetLevel;
            player.xp = 0;
            player.xpToNext = XpToNextForLevel(player.level);
            AddDevConsoleLine("Level set from " + std::to_string(previousLevel) + " to " + std::to_string(player.level) + ".");
            AddDevConsoleLine("XP reset to 0 / " + std::to_string(player.xpToNext) + " for level continuity.");
            AddDevConsoleLine("Passive draft checks were skipped.");
            AddLog("[DEV] Set level to " + std::to_string(player.level) + ".");
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
                        spawned                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          += 1;
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
        return rng.NextInt(minValue, maxValue);
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
            return t == TileType::Wall || t == TileType::WallFloor || t == TileType::Outside;
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
        engine::Rect ar{a.x, a.y, a.w, a.h};
        engine::Rect br{b.x, b.y, b.w, b.h};
        return engine::RoomsOverlap(ar, br);
    }

    std::pair<int, int> RoomCenter(const Room& room) const {
        engine::Rect rr{room.x, room.y, room.w, room.h};
        return engine::RoomCenter(rr);
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
        engine::Rect rr{room.x, room.y, room.w, room.h};
        engine::CarveRoom(rr, [&](int x, int y) {
            tiles[Index(x, y)] = TileType::Floor;
        });
    }

    void CarveCorridor(int x1, int y1, int x2, int y2) {
        engine::CarveCorridor(x1, y1, x2, y2, [&](int x, int y) {
            tiles[Index(x, y)] = TileType::Floor;
        });
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
                    enemy.mana = 5;

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

        int solidVariantCount = SolidDecorationVariantCount();

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

    int XpToNextForLevel(int level) const {
        int clampedLevel = std::max(1, level);
        int xpToNext = 10;
        for (int currentLevel = 1; currentLevel < clampedLevel; ++currentLevel) {
            xpToNext = static_cast<int>(std::round(xpToNext * 1.4f));
        }
        return std::max(1, xpToNext);
    }

    void ApplySingleLevelUp(bool checkPassiveDraft) {
        player.level += 1;
        player.skillPoints += 1;
        player.xpToNext = static_cast<int>(std::round(player.xpToNext * 1.4f));
        player.maxHp += 4;
        player.hp = std::min(player.maxHp, player.hp + 4);
        player.atk += 1;
        AddLog("Level up! You are now level " + std::to_string(player.level) + ".");
        AddLog("Skill point gained. Press Tab to open Character Menu.");
        if (checkPassiveDraft) {
            pendingPassiveChoices += 1;
            AddLog("Passive draft unlocked.");
        }
        RecomputePlayerStats();
    }

    void GainXp(int amount) {
        PassiveRuntimeEffects effects = ComputePassiveEffects();
        int adjustedAmount = std::max(0, static_cast<int>(std::round(static_cast<float>(amount) * std::max(0.05f, effects.expMult))));
        player.xp += adjustedAmount;
        while (player.xp >= player.xpToNext) {
            player.xp -= player.xpToNext;
            ApplySingleLevelUp(true);
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

        if (!IsWalkable(targetX, targetY)) {
            TileType blockedTile = tiles[Index(targetX, targetY)];
            if (blockedTile == TileType::Wall || blockedTile == TileType::WallFloor || blockedTile == TileType::Outside) {
                AddLog("Blocked by a wall.");
            } else if (IsSolidDecorationAt(targetX, targetY)) {
                AddLog("Blocked by debris.");
            } else if (IsBlockingInteractableAt(targetX, targetY)) {
                AddLog("Blocked by an unopened object.");
            }
            return false;
        }

        player.x = targetX;
        player.y = targetY;
        EmitMovementPuffAtTile(player.x, player.y, 8);

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

    void DamagePlayerFromEnemy(Enemy& e, const std::string& sourceLabel) {
        if (player.x > e.x) e.facingRight = true;
        else if (player.x < e.x) e.facingRight = false;
        StartEnemyAttackBumpToward(e, player.x, player.y);

        if (player.wizardStasisTurns > 0) {
            AddLog(sourceLabel + " cannot harm you while in Stasis.");
            return;
        }

        if (player.wizardMirrorImages > 0 && RollChance(0.50f)) {
            player.wizardMirrorImages -= 1;
            AddLog(sourceLabel + " strikes a Mirror Image.");
            if (player.wizardMirrorImages == 0) {
                AddLog("Your Mirror Images are gone.");
            }
            return;
        }

        float dodgeChance = std::clamp(0.08f + static_cast<float>(player.dexterity) * 0.012f, 0.08f, 0.40f);
        if (player.hunkerDownReady) {
            dodgeChance = std::clamp(dodgeChance + 0.12f, 0.0f, 0.75f);
        }

        if (RollChance(dodgeChance)) {
            AddLog(sourceLabel + " misses you.");
            if (player.parryRiposteTurns > 0) {
                for (int i = 0; i < static_cast<int>(enemies.size()); ++i) {
                    Enemy& enemy = enemies[i];
                    if (!enemy.alive) continue;
                    if (enemy.x == e.x && enemy.y == e.y) {
                        DamageEnemyByPlayer(i, std::max(1, player.atk / 2));
                        AddLog("Riposte counters " + sourceLabel + "!");
                        break;
                    }
                }
            }
            return;
        }

        int effectiveAtk = std::max(1, e.atk - std::max(0, e.armorBreakAmount));
        int rawDamage = RollDamage(effectiveAtk);
        int damageTaken = std::max(1, rawDamage - player.armor);

        if (player.wizardManaShieldTurns > 0 && player.mana > 0 && damageTaken > 0) {
            int absorbTarget = std::max(1, damageTaken / 2);
            int absorbed = std::min(absorbTarget, player.mana);
            player.mana -= absorbed;
            damageTaken -= absorbed;
            if (absorbed > 0) {
                AddLog("Mana Shield absorbs " + std::to_string(absorbed) + " damage.");
            }
        }

        player.hp -= damageTaken;
        EmitAttackParticlesAtTile(player.x, player.y, Color{255, 84, 84, 255}, 11);
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
            if (player.unstoppableTurns > 0) return;
            if (!player.poisoned) {
                player.poisoned = true;
                TriggerPoisonAfflictFlash();
                AddLog("You are Poisoned!");
            }
        } else if (e.archetype == EnemyArchetype::Wolf) {
            if (player.unstoppableTurns > 0) return;
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
        if (e.telegraphDx > 0) e.facingRight = true;
        else if (e.telegraphDx < 0) e.facingRight = false;
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
        if (picked.first > e.x) e.facingRight = true;
        else if (picked.first < e.x) e.facingRight = false;
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
        if (nx == enemy.x && ny == enemy.y) return;
        if (nx > enemy.x) enemy.facingRight = true;
        else if (nx < enemy.x) enemy.facingRight = false;
        enemy.x = nx;
        enemy.y = ny;
        EmitMovementPuffAtTile(enemy.x, enemy.y, 5);
        TryTriggerSpikeAtEnemy(enemy);
    }

    void ResolveEnemyTelegraph(Enemy& e) {
        const std::string name = EnemyName(e.archetype);
        if (e.telegraph == EnemyTelegraph::RangerLine) {
            std::vector<std::pair<int, int>> path;
            int x = e.x + e.telegraphDx;
            int y = e.y + e.telegraphDy;
            while (InBounds(x, y) && !IsTileOpaque(x, y)) {
                path.push_back({x, y});
                if (x == player.x && y == player.y) {
                    break;
                }
                x += e.telegraphDx;
                y += e.telegraphDy;
            }
            EmitProjectileTrailAlongTilesFrom(e.x, e.y, path, Color{252, 228, 120, 255}, 3);
            if (PlayerInRangerLine(e)) {
                DamagePlayerFromEnemy(e, name);
            } else {
                AddLog(name + "'s shot misses.");
            }
        } else if (e.telegraph == EnemyTelegraph::WizardPlus) {
            EmitProjectileTrailToTile(e.x, e.y, e.telegraphTargetX, e.telegraphTargetY, Color{155, 190, 255, 255}, 4, true, 48.0f, 96.0f, 0.12f, 0.26f, 1.8f, 3.6f);
            const int plus[5][2] = {{0,0},{1,0},{-1,0},{0,1},{0,-1}};
            for (const auto& offset : plus) {
                int tx = e.telegraphTargetX + offset[0];
                int ty = e.telegraphTargetY + offset[1];
                if (!InBounds(tx, ty)) continue;
                EmitAttackParticlesAtTile(tx, ty, Color{205, 228, 255, 255}, 5);
            }
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

            if (player.wizardRepulsionTurns > 0) {
                int nearDist = std::abs(e.x - player.x) + std::abs(e.y - player.y);
                if (nearDist == 1) {
                    int pushX = e.x + (e.x - player.x);
                    int pushY = e.y + (e.y - player.y);
                    if (IsWalkable(pushX, pushY) && !IsEnemyAt(pushX, pushY, i) && !(pushX == player.x && pushY == player.y)) {
                        MoveEnemyAndResolveFloorActions(e, pushX, pushY);
                        AddLog(std::string(EnemyName(e.archetype)) + " is pushed back by Repulsion Field.");
                        continue;
                    }
                }
            }

            if (player.rangerCounterTrapArmed) {
                int trapDist = std::abs(e.x - player.x) + std::abs(e.y - player.y);
                if (trapDist <= 1) {
                    int trapDamage = std::max(1, static_cast<int>(std::round(static_cast<float>(e.maxHp) * 0.25f)));
                    e.hp -= trapDamage;
                    e.grappledTurns = std::max(e.grappledTurns, 2);
                    player.rangerCounterTrapArmed = false;
                    player.rangerCounterTrapTurns = 0;
                    AddLog("Counter-Trap roots " + std::string(EnemyName(e.archetype)) + " for " + std::to_string(trapDamage) + " damage.");
                    if (e.hp <= 0) {
                        e.hp = 0;
                        e.alive = false;
                        score += 10;
                        GainXp(5 + dungeonLevel);
                        PlaySfx(sfxDeath, sfxDeathLoaded);
                        AddLog("Enemy defeated.");
                        MaybeDropItemFromEnemy();
                    }
                    continue;
                }
            }

            if (e.stunnedTurns > 0) {
                AddLog(std::string(EnemyName(e.archetype)) + " is stunned.");
                continue;
            }
            if (e.grappledTurns > 0) {
                AddLog(std::string(EnemyName(e.archetype)) + " is grappled.");
                continue;
            }

            e.isFleeingNow = false;

            if (e.telegraph != EnemyTelegraph::None) {
                ResolveEnemyTelegraph(e);
                if (IsWizardArchetype(e.archetype)) {
                    e.mana = std::min(5, e.mana + 1);
                }
                continue;
            }

            const int dist = std::abs(e.x - player.x) + std::abs(e.y - player.y);
            std::vector<uint8_t> fovMask = engine::ComputeFovMask(
                kMapWidth,
                kMapHeight,
                e.x,
                e.y,
                kEnemyViewRange,
                [&](int x, int y) {
                    TileType blockTile = tiles[Index(x, y)];
                    return blockTile == TileType::Wall || blockTile == TileType::WallFloor;
                });
            bool canSeePlayer = false;
            if (InBounds(player.x, player.y)) {
                canSeePlayer = fovMask[static_cast<size_t>(Index(player.x, player.y))] != 0;
            }
            if (canSeePlayer && player.rangerCamouflageTurns > 0 && dist > 1 && IsAdjacentToCover(player.x, player.y)) {
                canSeePlayer = false;
            }
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
                    if (e.mana >= 5) {
                        if (StartWizardTelegraph(e)) {
                            e.mana = 0;
                            AddLog(std::string(EnemyName(e.archetype)) + " begins a spell!");
                        } else {
                            RandomRoamEnemy(e, i);
                            e.mana = std::min(5, e.mana + 1);
                        }
                    } else {
                        RandomRoamEnemy(e, i);
                        e.mana = std::min(5, e.mana + 1);
                    }
                } else {
                    RandomRoamEnemy(e, i);
                    e.mana = std::min(5, e.mana + 1);
                }
                continue;
            }
        }
    }

    bool RollChance(float p) {
        return rng.RollChance(p);
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
                    if (tile == TileType::Wall || tile == TileType::WallFloor) DrawRectangleRec(rect, Color{18, 18, 25, 255});
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
                            DrawRectangleLines(x * kTileSize + 7, y * kTileSize + 7, kTileSize - 14, kTileSize - 14, Color{160, 150, 135, 220});
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
        if (characterMenuOpen || gameOver || classSelectionPending) return;

        if (player.armedActiveSkillSlot >= 0) {
            std::vector<std::pair<int, int>> preview = GetActiveSkillPreviewTiles(player.armedActiveSkillSlot);
            Color color = Color{120, 210, 255, 125};
            for (const auto& tile : preview) {
                int x = tile.first;
                int y = tile.second;
                if (!InBounds(x, y)) continue;
                DrawRectangle(x * kTileSize + 2, y * kTileSize + 2, kTileSize - 4, kTileSize - 4, color);
                DrawRectangleLines(x * kTileSize + 2, y * kTileSize + 2, kTileSize - 4, kTileSize - 4, Fade(color, 0.9f));
            }

            if (preview.empty()) {
                int targetTileX = 0;
                int targetTileY = 0;
                if (TryGetMouseWorldTile(targetTileX, targetTileY) && InBounds(targetTileX, targetTileY)) {
                    Color blocked = Color{220, 70, 70, 78};
                    int px = targetTileX * kTileSize + 4;
                    int py = targetTileY * kTileSize + 4;
                    int pw = kTileSize - 8;
                    int ph = kTileSize - 8;
                    DrawRectangle(px, py, pw, ph, blocked);
                    DrawRectangleLines(px, py, pw, ph, Fade(blocked, 0.95f));
                }
            }
            return;
        }

        std::vector<std::pair<int, int>> preview = GetAttackPreviewTiles();
        WeaponType weaponType = CurrentWeaponType();

        for (const auto& tile : preview) {
            int x = tile.first;
            int y = tile.second;
            if (!InBounds(x, y)) continue;

            Color color = Color{255, 120, 120, 120};
            if (weaponType == WeaponType::Bow) color = Color{120, 180, 255, 120};
            if (weaponType == WeaponType::Staff) color = player.mana >= PlayerMaxMana() ? Color{180, 120, 255, 130} : Color{100, 100, 100, 110};

            DrawRectangle(x * kTileSize + 2, y * kTileSize + 2, kTileSize - 4, kTileSize - 4, color);
            DrawRectangleLines(x * kTileSize + 2, y * kTileSize + 2, kTileSize - 4, kTileSize - 4, Fade(color, 0.9f));
        }
    }

    void DrawEnemyTelegraphs() const {
        if (characterMenuOpen || gameOver || classSelectionPending) return;

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
        float animationTime = static_cast<float>(GetTime());

        // Draw enemies
        for (const Enemy& e : enemies) {
            if (!e.alive) continue;

            int ex = e.x * kTileSize;
            int ey = e.y * kTileSize;
            float verticalOffset = EntityVerticalOffset(e.x, e.y, animationTime, e.isFlying);
            float horizontalOffset = EntityHorizontalOffset(e.x, e.y, animationTime, e.isFlying);
            float rockAngle = EntityRockAngle(e.x, e.y, animationTime);
            Vector2 attackBump = AttackBumpOffset(e.attackBumpDx, e.attackBumpDy, e.attackBumpTimer);

            Rectangle enemyIconSrc{};
            if (TryGetEnemyIconSource(e.iconVariant, enemyIconSrc)) {
                float enemyW = static_cast<float>(kTileSize - 6);
                float enemyCenterX = static_cast<float>(ex) + static_cast<float>(kTileSize) * 0.5f + horizontalOffset + attackBump.x;
                float enemyCenterY = static_cast<float>(ey) + static_cast<float>(kTileSize) * 0.5f + verticalOffset + attackBump.y;
                Rectangle enemyDst{enemyCenterX, enemyCenterY, enemyW, enemyW};
                Vector2 enemyOrigin{enemyW * 0.5f, enemyDst.height * 0.5f};
                DrawTexturePro(enemyAtlas, enemyIconSrc, enemyDst, enemyOrigin, rockAngle, WHITE);
            } else {
                float enemyFallbackW = static_cast<float>(kTileSize - 12);
                float enemyCenterX = static_cast<float>(ex) + static_cast<float>(kTileSize) * 0.5f + horizontalOffset + attackBump.x;
                float enemyCenterY = static_cast<float>(ey) + static_cast<float>(kTileSize) * 0.5f + verticalOffset + attackBump.y;
                Rectangle enemyRect{enemyCenterX, enemyCenterY, enemyFallbackW, enemyFallbackW};
                DrawRectanglePro(enemyRect, Vector2{enemyRect.width * 0.5f, enemyRect.height * 0.5f}, rockAngle, MAROON);
            }

            // Status marker above enemy
            const int cx = ex + kTileSize / 2;
            const int yAbove = std::max(0, static_cast<int>(std::round(static_cast<float>(ey) + verticalOffset)) - 20);

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
            float merchantOffset = EntityVerticalOffset(merchantX, merchantY, animationTime, false);
            float merchantShuffle = EntityHorizontalOffset(merchantX, merchantY, animationTime, false);
            float merchantAngle = EntityRockAngle(merchantX, merchantY, animationTime);
            float merchantCenterX = static_cast<float>(mx) + static_cast<float>(kTileSize) * 0.5f + merchantShuffle;
            float merchantCenterY = static_cast<float>(my) + static_cast<float>(kTileSize) * 0.5f + merchantOffset;
            Rectangle npcSrc{};
            if (TryGetNpcIconSource(merchantIconVariant, npcSrc)) {
                float npcW = static_cast<float>(kTileSize - 6);
                Rectangle npcDst{merchantCenterX, merchantCenterY, npcW, npcW};
                Vector2 npcOrigin{npcDst.width * 0.5f, npcDst.height * 0.5f};
                DrawTexturePro(npcAtlas, npcSrc, npcDst, npcOrigin, merchantAngle, WHITE);
            } else {
                float npcFallbackW = static_cast<float>(kTileSize - 8);
                Rectangle npcRect{merchantCenterX, merchantCenterY, npcFallbackW, npcFallbackW};
                DrawRectanglePro(npcRect, Vector2{npcRect.width * 0.5f, npcRect.height * 0.5f}, merchantAngle, Color{210, 160, 62, 255});
                const int glyphSize = 24;
                int glyphW = MeasureText("$", glyphSize);
                int glyphX = static_cast<int>(std::round(merchantCenterX - static_cast<float>(glyphW) * 0.5f));
                int glyphY = static_cast<int>(std::round(merchantCenterY - static_cast<float>(glyphSize) * 0.5f));
                DrawText("$", glyphX, glyphY, glyphSize, Color{36, 28, 18, 255});
            }
        }

        // Draw player last (on top)
        int px = player.x * kTileSize;
        int py = player.y * kTileSize;
        float playerOffset = EntityVerticalOffset(player.x, player.y, animationTime, false);
        float playerShuffle = EntityHorizontalOffset(player.x, player.y, animationTime, false);
        float playerAngle = EntityRockAngle(player.x, player.y, animationTime);
        Vector2 playerAttackBump = AttackBumpOffset(player.attackBumpDx, player.attackBumpDy, player.attackBumpTimer);
        Rectangle playerIconSrc{};
        if (TryGetClassIconSource(player.playerClass, playerIconSrc)) {
            float playerW = static_cast<float>(kTileSize - 6);
            float playerCenterX = static_cast<float>(px) + static_cast<float>(kTileSize) * 0.5f + playerShuffle + playerAttackBump.x;
            float playerCenterY = static_cast<float>(py) + static_cast<float>(kTileSize) * 0.5f + playerOffset + playerAttackBump.y;
            Rectangle playerDst{playerCenterX, playerCenterY, playerW, playerW};
            Vector2 playerOrigin{playerW * 0.5f, playerDst.height * 0.5f};
            DrawTexturePro(playerAtlas, playerIconSrc, playerDst, playerOrigin, playerAngle, WHITE);
        } else {
            Color fallback = SKYBLUE;
            if (player.playerClass == PlayerClass::Warrior) fallback = Color{200, 90, 90, 255};
            if (player.playerClass == PlayerClass::Ranger) fallback = Color{80, 180, 110, 255};
            if (player.playerClass == PlayerClass::Wizard) fallback = Color{120, 150, 230, 255};
            float playerFallbackW = static_cast<float>(kTileSize - 10);
            float playerCenterX = static_cast<float>(px) + static_cast<float>(kTileSize) * 0.5f + playerShuffle + playerAttackBump.x;
            float playerCenterY = static_cast<float>(py) + static_cast<float>(kTileSize) * 0.5f + playerOffset + playerAttackBump.y;
            Rectangle playerRect{playerCenterX, playerCenterY, playerFallbackW, playerFallbackW};
            DrawRectanglePro(playerRect, Vector2{playerRect.width * 0.5f, playerRect.height * 0.5f}, playerAngle, fallback);
        }
    }

    void DrawCharacterMenuTabs() const {
        for (int i = 0; i < 3; ++i) {
            CharacterMenuTab tab = static_cast<CharacterMenuTab>(i);
            bool active = (tab == characterMenuTab);
            Rectangle rect = CharacterMenuTabRect(i);
            DrawRectangleRec(rect, active ? Color{66, 78, 110, 235} : Color{28, 30, 38, 220});
            DrawRectangleLinesEx(rect, 2.0f, active ? Color{190, 220, 255, 255} : Color{90, 96, 116, 255});

            const char* label = CharacterMenuTabLabel(tab);
            int tw = MeasureText(label, 24);
            DrawText(label, static_cast<int>(rect.x + (rect.width - static_cast<float>(tw)) * 0.5f), static_cast<int>(rect.y + 10.0f), 24, active ? Color{230, 240, 255, 255} : LIGHTGRAY);
        }
    }

    void DrawStatsOverlay() const {
        DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.74f));

        const int panelW = 1020;
        const int panelH = 620;
        const int panelX = (kScreenWidth - panelW) / 2;
        const int panelY = (kScreenHeight - panelH) / 2;

        DrawRectangle(panelX, panelY, panelW, panelH, Color{16, 16, 24, 248});
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{90, 100, 125, 255});

        DrawText("Stats", panelX + 24, panelY + 14, 36, RAYWHITE);
        const char* statsSkillPointsText = TextFormat("Skill Points: %i", player.skillPoints);
        int statsSkillPointsW = MeasureText(statsSkillPointsText, 28);
        DrawText(statsSkillPointsText, panelX + panelW - 24 - statsSkillPointsW, panelY + 20, 28, Color{255, 220, 120, 255});
        DrawText("Spend points to raise base attributes.", panelX + 24, panelY + 56, 22, LIGHTGRAY);

        const int statsX = panelX + 34;
        const int statsY = panelY + 154;
        const int rowW = 420;
        const int rowH = 54;
        const int labelX = statsX + 16;
        const int valueX = statsX + 180;

        const int strGear = EquippedStrengthBonus();
        const int dexGear = EquippedDexterityBonus();
        const int staGear = EquippedStaminaBonus();

        for (int i = 0; i < 3; ++i) {
            Rectangle rowRect{static_cast<float>(statsX), static_cast<float>(statsY + i * rowH), static_cast<float>(rowW), static_cast<float>(rowH - 6)};
            bool selected = (i == statsAttributeSelection);
            DrawRectangleRec(rowRect, selected ? Color{58, 74, 108, 220} : Color{28, 32, 44, 215});
            DrawRectangleLinesEx(rowRect, 2.0f, selected ? Color{190, 220, 255, 255} : Color{82, 86, 96, 255});
        }

        DrawText("Strength", labelX, statsY + 12, 24, Color{255, 160, 160, 255});
        DrawText(TextFormat("%i (%+i gear)", player.baseStrength, strGear), valueX, statsY + 12, 24, RAYWHITE);

        DrawText("Dexterity", labelX, statsY + rowH + 12, 24, Color{150, 230, 170, 255});
        DrawText(TextFormat("%i (%+i gear)", player.baseDexterity, dexGear), valueX, statsY + rowH + 12, 24, RAYWHITE);

        DrawText("Stamina", labelX, statsY + rowH * 2 + 12, 24, Color{170, 200, 255, 255});
        DrawText(TextFormat("%i (%+i gear)", player.baseMaxHp, staGear), valueX, statsY + rowH * 2 + 12, 24, RAYWHITE);

        DrawText("Passive Buffs", panelX + 500, panelY + 118, 30, Color{205, 225, 255, 255});
        Rectangle passivePanel{static_cast<float>(panelX + 500), static_cast<float>(panelY + 154), static_cast<float>(panelW - 534), static_cast<float>(panelH - 216)};
        DrawRectangleRec(passivePanel, Color{24, 26, 34, 225});
        DrawRectangleLinesEx(passivePanel, 2.0f, Color{78, 82, 94, 255});

        int py = panelY + 168;
        if (player.passiveSkills.empty()) {
            DrawText("No passive buffs yet.", panelX + 516, py, 20, GRAY);
        } else {
            for (size_t i = 0; i < player.passiveSkills.size(); ++i) {
                const GeneratedPassive& passive = player.passiveSkills[i];
                std::string title = PassiveSkillName(passive);
                std::string effect = PassiveStatEffectText(passive) + " | " + PassiveSuffixEffectText(passive);

                DrawText(title.c_str(), panelX + 516, py, 18, ItemTierColor(passive.grade));
                py += 22;
                std::vector<std::string> lines = WrapTextStrict(effect, panelW - 560, 16);
                for (const std::string& line : lines) {
                    if (py > panelY + panelH - 64) break;
                    DrawText(line.c_str(), panelX + 516, py, 16, Color{188, 205, 235, 255});
                    py += 18;
                }
                py += 8;
                if (py > panelY + panelH - 64) {
                    DrawText("...", panelX + panelW - 52, panelY + panelH - 58, 24, Color{170, 180, 210, 255});
                    break;
                }
            }
        }

        DrawText("W/S: Select Stat   Enter/E/Space: Spend Point   Click tabs to switch   Tab/Esc: Close", panelX + 24, panelY + panelH - 22, 18, LIGHTGRAY);
    }

    void DrawCharacterMenuOverlay() const {
        if (characterMenuTab == CharacterMenuTab::Inventory) {
            DrawInventoryOverlay();
        } else if (characterMenuTab == CharacterMenuTab::SkillTree) {
            DrawSkillTreeOverlay();
        } else {
            DrawStatsOverlay();
        }

        DrawCharacterMenuTabs();
    }

    void DrawSkillTreeOverlay() const {
        DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.76f));
        Vector2 mousePos = GetMousePosition();

        const int panelW = 1020;
        const int panelH = 620;
        const int panelX = (kScreenWidth - panelW) / 2;
        const int panelY = (kScreenHeight - panelH) / 2;

        DrawRectangle(panelX, panelY, panelW, panelH, Color{16, 16, 24, 248});
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{90, 100, 125, 255});

        int unlockedSkillCount = TotalUnlockedTreeSkills();
        bool treeLocked = unlockedSkillCount >= kSkillTreeSkillCap;

        DrawText("Skill Tree", panelX + 24, panelY + 14, 36, RAYWHITE);
        const char* skillPointsText = TextFormat("Skill Points: %i", player.skillPoints);
        int skillPointsW = MeasureText(skillPointsText, 28);
        DrawText(skillPointsText, panelX + panelW - 24 - skillPointsW, panelY + 20, 28, Color{255, 220, 120, 255});
        const char* skillsCountText = TextFormat("Skills: %i/%i", unlockedSkillCount, kSkillTreeSkillCap);
        int skillsCountW = MeasureText(skillsCountText, 22);
        DrawText(skillsCountText, panelX + panelW - 24 - skillsCountW, panelY + 54, 22, treeLocked ? Color{255, 140, 140, 255} : Color{170, 210, 255, 255});
        DrawText("Unlock up to 10 Active Skills.", panelX + 24, panelY + 56, 21, LIGHTGRAY);
        if (treeLocked) {
            DrawText("Skill tree locked. Further power comes from passives and gear.", panelX + 24, panelY + 84, 20, Color{255, 160, 160, 255});
        }

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
        ActiveSkillId hoveredTreeSkill = ActiveSkillId::None;
        Rectangle hoveredTreeRect{};

        auto branchSkillForRank = [&](int branchIndex, int rank)->ActiveSkillId {
            if (player.playerClass == PlayerClass::Warrior) {
                return WarriorBranchSkill(branchIndex, rank);
            }
            if (player.playerClass == PlayerClass::Ranger) {
                return RangerBranchSkill(branchIndex, rank);
            }
            return WizardBranchSkill(branchIndex, rank);
        };

        for (int i = 0; i < kClassSkillCount; ++i) {
            int bx = leftBranchX + i * branchSpacing;
            int rank = player.classSkillRanks[i];
            bool selectedBranch = (i == skillSelection);

            DrawLineEx(
                Vector2{static_cast<float>(rootX), static_cast<float>(rootY + 12)},
                Vector2{static_cast<float>(bx), static_cast<float>(branchTopY - 18)},
                4.0f,
                Fade(branchColors[i], 0.75f)
            );

            DrawText(branchTitles[i], bx - 62, panelY + 152, 30, selectedBranch ? RAYWHITE : Fade(branchColors[i], 0.95f));

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
                Rectangle frameRect{static_cast<float>(bx - 16), static_cast<float>(nodeY - 16), 32.0f, 32.0f};
                bool nodeSelected = selectedBranch && (p + 1 == skillRankSelection);
                bool nodeHovered = CheckCollisionPointRec(mousePos, frameRect);
                DrawRectangleRec(frameRect, nodeFill);
                DrawRectangleLinesEx(frameRect, 2.0f, (nodeSelected || nodeHovered) ? Color{220, 230, 255, 255} : Color{100, 110, 130, 255});

                Rectangle iconRect{frameRect.x + 2.0f, frameRect.y + 2.0f, 28.0f, 28.0f};
                ActiveSkillId nodeSkill = branchSkillForRank(i, p + 1);
                Color iconColor = filled ? Fade(branchColors[i], 0.9f) : Color{58, 62, 72, 255};
                DrawRectangleRec(iconRect, iconColor);
                Texture2D nodeTexture{};
                if (TryGetActiveSkillIconTexture(nodeSkill, nodeTexture)) {
                    Rectangle src{0.0f, 0.0f, static_cast<float>(nodeTexture.width), static_cast<float>(nodeTexture.height)};
                    Rectangle dst{iconRect.x + 2.0f, iconRect.y + 2.0f, iconRect.width - 4.0f, iconRect.height - 4.0f};
                    DrawTexturePro(nodeTexture, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
                } else {
                    const char* iconTag = ActiveSkillIconTag(nodeSkill);
                    int tagW = MeasureText(iconTag, 12);
                    DrawText(iconTag, static_cast<int>(iconRect.x + (iconRect.width - static_cast<float>(tagW)) * 0.5f), static_cast<int>(iconRect.y + 8.0f), 12, WHITE);
                }

                if (nodeHovered) {
                    hoveredTreeSkill = nodeSkill;
                    hoveredTreeRect = frameRect;
                }
            }
        }

        if (hoveredTreeSkill != ActiveSkillId::None) {
            const char* title = ActiveSkillName(hoveredTreeSkill);
            const char* desc = ActiveSkillTooltip(hoveredTreeSkill);
            const int tipW = 430;
            const int tipH = 90;
            int tipX = static_cast<int>(hoveredTreeRect.x + hoveredTreeRect.width * 0.5f) - tipW / 2;
            int tipY = static_cast<int>(hoveredTreeRect.y) - tipH - 8;
            tipX = std::clamp(tipX, panelX + 12, panelX + panelW - tipW - 12);
            tipY = std::max(panelY + 12, tipY);

            DrawRectangle(tipX, tipY, tipW, tipH, Color{12, 14, 22, 238});
            DrawRectangleLinesEx(Rectangle{static_cast<float>(tipX), static_cast<float>(tipY), static_cast<float>(tipW), static_cast<float>(tipH)}, 2.0f, Color{95, 120, 165, 255});
            DrawText(title, tipX + 10, tipY + 8, 22, RAYWHITE);
            DrawText(desc, tipX + 10, tipY + 40, 18, Color{195, 210, 236, 255});
        }

        DrawText("Mouse: Hover/click node   A/D: Branch   W/S: Rank   Enter/E/Space: Spend   Click tabs to switch   Tab/Esc: Close", panelX + 24, panelY + panelH - 22, 18, LIGHTGRAY);
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
        Vector2 mousePos = GetMousePosition();

        for (int i = 0; i < 3; ++i) {
            int x = startX + i * (cardW + gap);
            Rectangle cardRect{static_cast<float>(x), static_cast<float>(cardY), static_cast<float>(cardW), static_cast<float>(cardH)};
            bool selected = (i == passiveChoiceSelection);
            bool hovered = CheckCollisionPointRec(mousePos, cardRect);

            DrawRectangleRec(cardRect, (selected || hovered) ? Color{56, 70, 106, 220} : Color{28, 32, 44, 215});

            const GeneratedPassive& passive = passiveChoiceOptions[static_cast<size_t>(i)];
            Color rarityColor = ItemTierColor(passive.grade);
            DrawRectangleLinesEx(cardRect, 2.0f, rarityColor);
            if (selected || hovered) {
                Rectangle innerRect{cardRect.x + 3.0f, cardRect.y + 3.0f, cardRect.width - 6.0f, cardRect.height - 6.0f};
                DrawRectangleLinesEx(innerRect, 2.0f, Color{195, 220, 255, 255});
            }

            std::string perkName = PassiveSkillName(passive);
            std::string statLine = PassiveStatEffectText(passive);
            std::string suffixLine = PassiveSuffixEffectText(passive);

            Texture2D passiveTexture{};
            if (TryGetPassiveIconTexture(passive.suffix, passiveTexture)) {
                Rectangle src{0.0f, 0.0f, static_cast<float>(passiveTexture.width), static_cast<float>(passiveTexture.height)};
                Rectangle dst{static_cast<float>(x + (cardW - 48) / 2), static_cast<float>(cardY + 44), 48.0f, 48.0f};
                DrawTexturePro(passiveTexture, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
            } else {
                Rectangle fallbackIcon{static_cast<float>(x + (cardW - 48) / 2), static_cast<float>(cardY + 44), 48.0f, 48.0f};
                DrawRectangleRec(fallbackIcon, Fade(rarityColor, 0.35f));
                DrawRectangleLinesEx(fallbackIcon, 2.0f, rarityColor);
            }

            DrawText(TextFormat("[%i]", i + 1), x + 12, cardY + 10, 26, LIGHTGRAY);
            DrawText(PassiveGradeName(passive.grade), x + cardW - 108, cardY + 14, 20, rarityColor);

            int textY = cardY + 100;
            int titleSize = 20;
            std::vector<std::string> titleLines = WrapTextStrict(perkName, cardW - 20, titleSize);
            if (titleLines.size() > 2) {
                titleLines.resize(2);
                titleSize = 18;
            }
            for (const std::string& line : titleLines) {
                int lineW = MeasureText(line.c_str(), titleSize);
                DrawText(line.c_str(), x + (cardW - lineW) / 2, textY, titleSize, RAYWHITE);
                textY += titleSize + 2;
            }

            textY += 6;
            int statSize = 17;
            std::vector<std::string> statLines = WrapTextStrict(statLine, cardW - 20, statSize);
            for (const std::string& line : statLines) {
                int lineW = MeasureText(line.c_str(), statSize);
                DrawText(line.c_str(), x + (cardW - lineW) / 2, textY, statSize, Color{210, 225, 250, 255});
                textY += statSize + 2;
            }

            int suffixSize = 15;
            std::vector<std::string> suffixLines = WrapTextStrict(suffixLine, cardW - 20, suffixSize);
            if (suffixLines.size() > 2) {
                suffixLines.resize(2);
            }
            for (const std::string& line : suffixLines) {
                int lineW = MeasureText(line.c_str(), suffixSize);
                DrawText(line.c_str(), x + (cardW - lineW) / 2, textY, suffixSize, Color{185, 205, 240, 255});
                textY += suffixSize + 2;
            }
        }

        DrawText("Mouse: Hover + Click to choose   Keyboard: A/D/W/S + Enter/E/Space", panelX + 24, panelY + panelH - 28, 22, LIGHTGRAY);
    }

    void DrawInventoryOverlay() const {
        DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.72f));
        Vector2 mousePos = GetMousePosition();

        const int panelW = 1020;
        const int panelH = 620;
        const int panelX = (kScreenWidth - panelW) / 2;
        const int panelY = (kScreenHeight - panelH) / 2;

        DrawRectangle(panelX, panelY, panelW, panelH, Color{16, 16, 24, 245});
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{90, 90, 120, 255});

        DrawText("Inventory", panelX + 20, panelY + 14, 34, RAYWHITE);
        DrawText("Arrows/W-S: Select   E/Enter: Equip or Use   Click tabs to switch   Tab/Esc: Close", panelX + 20, panelY + 54, 20, LIGHTGRAY);
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

    void DrawMainMenu() const {
        DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.90f));

        const int panelW = 520;
        const int panelH = 380;
        const int panelX = (kScreenWidth - panelW) / 2;
        const int panelY = (kScreenHeight - panelH) / 2;
        DrawRectangle(panelX, panelY, panelW, panelH, Color{18, 20, 30, 248});
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{90, 100, 125, 255});

        const char* title = "DONJON";
        int titleW = MeasureText(title, 52);
        DrawText(title, panelX + (panelW - titleW) / 2, panelY + 30, 52, RAYWHITE);

        const char* buttons[3] = {"Start Game", "Options", "Exit to Desktop"};
        const int buttonW = 360;
        const int buttonH = 52;
        const int buttonX = panelX + (panelW - buttonW) / 2;
        const int firstButtonY = panelY + 120;
        Vector2 mousePos = GetMousePosition();

        for (int i = 0; i < 3; ++i) {
            Rectangle rect{static_cast<float>(buttonX), static_cast<float>(firstButtonY + i * 64), static_cast<float>(buttonW), static_cast<float>(buttonH)};
            bool hovered = CheckCollisionPointRec(mousePos, rect);
            bool selected = (i == mainMenuSelection);
            DrawRectangleRec(rect, (hovered || selected) ? Color{58, 72, 106, 230} : Color{30, 34, 44, 225});
            DrawRectangleLinesEx(rect, 2.0f, (hovered || selected) ? Color{195, 220, 255, 255} : Color{95, 102, 118, 255});

            int tw = MeasureText(buttons[i], 26);
            DrawText(buttons[i], buttonX + (buttonW - tw) / 2, firstButtonY + i * 64 + 13, 26, RAYWHITE);
        }

        DrawText("Use mouse or W/S + Enter", panelX + 130, panelY + panelH - 34, 22, LIGHTGRAY);
    }

    void DrawPauseMenu() const {
        DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.78f));

        const int panelW = 560;
        const int panelH = 420;
        const int panelX = (kScreenWidth - panelW) / 2;
        const int panelY = (kScreenHeight - panelH) / 2;

        DrawRectangle(panelX, panelY, panelW, panelH, Color{18, 20, 30, 248});
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{90, 100, 125, 255});

        DrawText("Paused", panelX + 24, panelY + 20, 42, RAYWHITE);
        DrawText("Esc: Resume", panelX + panelW - 170, panelY + 34, 22, LIGHTGRAY);

        const char* buttons[4] = {"Resume", "Options", "Exit to Main Menu", "Exit to Desktop"};
        const int buttonW = 360;
        const int buttonH = 52;
        const int buttonX = panelX + (panelW - buttonW) / 2;
        const int firstButtonY = panelY + 100;
        Vector2 mousePos = GetMousePosition();

        for (int i = 0; i < 4; ++i) {
            Rectangle rect{static_cast<float>(buttonX), static_cast<float>(firstButtonY + i * 64), static_cast<float>(buttonW), static_cast<float>(buttonH)};
            bool hovered = CheckCollisionPointRec(mousePos, rect);
            bool selected = (i == pauseMenuSelection);
            DrawRectangleRec(rect, (hovered || selected) ? Color{58, 72, 106, 230} : Color{30, 34, 44, 225});
            DrawRectangleLinesEx(rect, 2.0f, (hovered || selected) ? Color{195, 220, 255, 255} : Color{95, 102, 118, 255});

            int tw = MeasureText(buttons[i], 26);
            DrawText(buttons[i], buttonX + (buttonW - tw) / 2, firstButtonY + i * 64 + 13, 26, RAYWHITE);
        }
    }

    void DrawOptionsMenu() const {
        DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.84f));

        const int panelW = 900;
        const int panelH = 560;
        const int panelX = (kScreenWidth - panelW) / 2;
        const int panelY = (kScreenHeight - panelH) / 2;

        DrawRectangle(panelX, panelY, panelW, panelH, Color{18, 20, 30, 248});
        DrawRectangleLines(panelX, panelY, panelW, panelH, Color{90, 100, 125, 255});
        DrawText("Options", panelX + 24, panelY + 20, 42, RAYWHITE);

        Vector2 mousePos = GetMousePosition();
        for (int i = 0; i < 4; ++i) {
            OptionsTab tab = static_cast<OptionsTab>(i);
            Rectangle tabRect{static_cast<float>(panelX + 24 + i * 210), static_cast<float>(panelY + 74), 190.0f, 42.0f};
            bool hovered = CheckCollisionPointRec(mousePos, tabRect);
            bool selected = (tab == optionsTab);
            DrawRectangleRec(tabRect, (hovered || selected) ? Color{58, 72, 106, 230} : Color{30, 34, 44, 225});
            DrawRectangleLinesEx(tabRect, 2.0f, (hovered || selected) ? Color{195, 220, 255, 255} : Color{95, 102, 118, 255});

            const char* label = OptionsTabLabel(tab);
            int tw = MeasureText(label, 24);
            DrawText(label, static_cast<int>(tabRect.x + (tabRect.width - static_cast<float>(tw)) * 0.5f), static_cast<int>(tabRect.y + 9.0f), 24, RAYWHITE);
        }

        if (optionsTab == OptionsTab::Graphics) {
            DrawText("Resolution", panelX + 90, panelY + 165, 30, RAYWHITE);
            DrawText("<", panelX + 98, panelY + 188, 34, LIGHTGRAY);
            DrawText(">", panelX + 548, panelY + 188, 34, LIGHTGRAY);
            const ResolutionOption& res = kResolutionOptions[static_cast<size_t>(std::clamp(selectedResolutionIndex, 0, static_cast<int>(kResolutionOptions.size()) - 1))];
            int resW = MeasureText(res.label, 28);
            DrawText(res.label, panelX + 340 - resW / 2, panelY + 194, 28, Color{180, 220, 255, 255});

            DrawText("Window", panelX + 90, panelY + 245, 30, RAYWHITE);
            DrawText("<", panelX + 98, panelY + 268, 34, LIGHTGRAY);
            DrawText(">", panelX + 548, panelY + 268, 34, LIGHTGRAY);
            const char* mode = WindowModeLabel(windowModeSetting);
            int modeW = MeasureText(mode, 28);
            DrawText(mode, panelX + 340 - modeW / 2, panelY + 274, 28, Color{180, 220, 255, 255});

        } else if (optionsTab == OptionsTab::Audio) {
            DrawText("BGM", panelX + 90, panelY + 176, 30, RAYWHITE);
            Rectangle bgmBar{static_cast<float>(panelX + 180), static_cast<float>(panelY + 190), 320.0f, 20.0f};
            DrawRectangleRec(bgmBar, Color{30, 34, 44, 225});
            DrawRectangle(static_cast<int>(bgmBar.x), static_cast<int>(bgmBar.y), static_cast<int>(bgmBar.width * (static_cast<float>(bgmVolumeLevel) / 10.0f)), static_cast<int>(bgmBar.height), Color{120, 200, 255, 255});
            DrawRectangleLinesEx(bgmBar, 2.0f, Color{95, 102, 118, 255});
            DrawText(TextFormat("%i", bgmVolumeLevel), panelX + 520, panelY + 186, 28, LIGHTGRAY);

            DrawText("SFX", panelX + 90, panelY + 256, 30, RAYWHITE);
            Rectangle sfxBar{static_cast<float>(panelX + 180), static_cast<float>(panelY + 270), 320.0f, 20.0f};
            DrawRectangleRec(sfxBar, Color{30, 34, 44, 225});
            DrawRectangle(static_cast<int>(sfxBar.x), static_cast<int>(sfxBar.y), static_cast<int>(sfxBar.width * (static_cast<float>(sfxVolumeLevel) / 10.0f)), static_cast<int>(sfxBar.height), Color{120, 200, 255, 255});
            DrawRectangleLinesEx(sfxBar, 2.0f, Color{95, 102, 118, 255});
            DrawText(TextFormat("%i", sfxVolumeLevel), panelX + 520, panelY + 266, 28, LIGHTGRAY);
        } else if (optionsTab == OptionsTab::Controls) {
            int y = panelY + 170;
            DrawText("WASD / Arrows: Move", panelX + 90, y, 28, LIGHTGRAY); y += 36;
            DrawText("Mouse Left: Preview + Attack", panelX + 90, y, 28, LIGHTGRAY); y += 36;
            DrawText("Tab: Character Menu", panelX + 90, y, 28, LIGHTGRAY); y += 36;
            DrawText("E: Interact", panelX + 90, y, 28, LIGHTGRAY); y += 36;
            DrawText("Esc: Pause Menu", panelX + 90, y, 28, LIGHTGRAY); y += 36;
            DrawText("1..0: Select Active Skill Slot", panelX + 90, y, 28, LIGHTGRAY); y += 36;
        }
    }

    void DrawHud() const {
        const int mapW = kMapWidth * kTileSize;
        const int mapH = kMapHeight * kTileSize;

        const int leftX = 12;
        const int statBarW = 238;
        const int statBarH = 18;

        const char* hpText = TextFormat("HP: %i/%i", player.hp, player.maxHp);
        DrawRectangle(leftX, 10, statBarW, statBarH, Color{34, 28, 30, 230});
        float hpRatio = (player.maxHp > 0) ? static_cast<float>(player.hp) / static_cast<float>(player.maxHp) : 0.0f;
        hpRatio = std::clamp(hpRatio, 0.0f, 1.0f);
        DrawRectangle(leftX + 1, 11, static_cast<int>(std::floor((statBarW - 2) * hpRatio)), statBarH - 2, Color{220, 64, 64, 255});
        DrawRectangleLines(leftX, 10, statBarW, statBarH, Color{160, 110, 110, 255});
        int hpTextW = MeasureText(hpText, 14);
        DrawText(hpText, leftX + (statBarW - hpTextW) / 2, 12, 14, RAYWHITE);

        const char* mpText = TextFormat("MP: %i/%i", player.mana, PlayerMaxMana());
        DrawRectangle(leftX, 34, statBarW, statBarH, Color{26, 30, 38, 230});
        float mpRatio = (PlayerMaxMana() > 0) ? static_cast<float>(player.mana) / static_cast<float>(PlayerMaxMana()) : 0.0f;
        mpRatio = std::clamp(mpRatio, 0.0f, 1.0f);
        DrawRectangle(leftX + 1, 35, static_cast<int>(std::floor((statBarW - 2) * mpRatio)), statBarH - 2, Color{70, 130, 235, 255});
        DrawRectangleLines(leftX, 34, statBarW, statBarH, Color{115, 140, 185, 255});
        int mpTextW = MeasureText(mpText, 14);
        DrawText(mpText, leftX + (statBarW - mpTextW) / 2, 36, 14, RAYWHITE);

        const char* floorText = TextFormat("Floor: %i", dungeonLevel);
        int floorW = MeasureText(floorText, 24);
        DrawText(floorText, kScreenWidth - floorW - 12, 10, 24, LIGHTGRAY);

        const char* goldText = TextFormat("Gold: %i", player.gold);
        int goldW = MeasureText(goldText, 20);
        DrawText(goldText, kScreenWidth - goldW - 12, 38, 20, Color{255, 220, 120, 255});

        const int levelY = mapH - 108;
        const char* levelText = TextFormat("Level: %i", player.level);
        int levelW = MeasureText(levelText, 18);
        DrawText(levelText, mapW / 2 - levelW / 2, levelY, 18, Color{130, 210, 255, 255});

        const char* expText = "EXP";
        int expW = MeasureText(expText, 18);
        DrawText(expText, mapW / 2 - expW / 2, levelY + 40, 18, LIGHTGRAY);

        const int barW = 10 * 32 + 9 * 6;
        const int barH = 12;
        const int barX = mapW / 2 - barW / 2;
        const int barY = levelY + 60;
        DrawRectangle(barX, barY, barW, barH, Color{30, 34, 44, 230});

        float ratio = (player.xpToNext > 0) ? static_cast<float>(player.xp) / static_cast<float>(player.xpToNext) : 0.0f;
        ratio = std::clamp(ratio, 0.0f, 1.0f);
        DrawRectangle(barX + 1, barY + 1, static_cast<int>(std::floor((barW - 2) * ratio)), barH - 2, Color{88, 210, 105, 255});
        DrawRectangleLines(barX, barY, barW, barH, Color{160, 180, 210, 255});
        const char* expValueText = TextFormat("%i/%i", player.xp, player.xpToNext);
        int expValueW = MeasureText(expValueText, 10);
        DrawText(expValueText, barX + (barW - expValueW) / 2, barY + 1, 10, RAYWHITE);
    }
};


class GameRuntime::Impl {
public:
    Game game;
};

GameRuntime::GameRuntime()
    : impl(std::make_unique<Impl>()) {
}

GameRuntime::~GameRuntime() = default;

int GameRuntime::ScreenWidth() const {
    return Game::kScreenWidth;
}

int GameRuntime::ScreenHeight() const {
    return Game::kScreenHeight;
}

bool GameRuntime::ShouldExitRequested() const {
    return impl->game.ShouldExitRequested();
}

void GameRuntime::LoadSettingsFromFile() {
    impl->game.LoadSettingsFromFile();
}

void GameRuntime::ApplyLoadedSettings() {
    impl->game.ApplyLoadedSettings();
}

void GameRuntime::LoadAssets() {
    impl->game.LoadAssets();
}

void GameRuntime::UnloadAssets() {
    impl->game.UnloadAssets();
}

void GameRuntime::Update(const engine::FrameContext& frame) {
    impl->game.Update(frame);
}

void GameRuntime::Draw() const {
    impl->game.Draw();
}
