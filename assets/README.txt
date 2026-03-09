Place your inventory icon atlas here as:
assets/icons/item_atlas.png

Expected layout:
- Cell size: 32x32 pixels per icon
- Grid used by the game:
    Row 0: [bronzeSword, ironSword, steelSword, mithrilSword, vorpalSword]
    Row 1: [bronzeBow, ironBow, steelBow, mithrilBow, VorpalSBow]
    Row 2: [bronzeStaff, ironStaff, steelStaff, mithrilStaff, vorpalStaff]
    Row 3: [metalHead, leatherHead, clothHead]
    Row 4: [metalTop, leatherTop, clothTop]
    Row 5: [metalHands, leatherHands, clothHands]
    Row 6: [metalFeet, leatherFeet, clothFeet]
    Row 7: [bronzeShield, ironShield, steelShield, mithrilShield, vorpalShield]
    Row 8: [bronzeRing, ironRing, steelRing, mithrilRing, vorpalRing]
    Row 9: [bronzeNeck, ironNeck, steelNeck, mitrhilNeck, vorpalNeck]
    Row 10: [healthPotion, antidotePotion, armorPotion, bandageWrap]
    row 11: [smallGold, mediumGold, largeGold, bagGold]

Any missing/out-of-bounds icon automatically falls back to the ASCII glyph in-game.


Room atlas:
assets/icons/room_atlas.png

Expected layout:
- Cell size: 32x32 pixels per tile
- The game picks a deterministic variant based on tile position, so multiple columns create natural variation.
- Stairs render on top of the floor tile.
- If this atlas is missing/invalid, the game falls back to flat color floor/wall rendering.
- If a wall has a floor below it, it uses a wall variant (dirtFloorWall for example).
- Grid used by the game:
    row 0: [dirtWall, dirtFloorWall]
    row 1: [dirtFloor, dirtFloor1, dirtFloor2, dirtFloor3]
    row 2: [dirtOutside, dirtOutside1, dirtOutside2, dirtOutside3]
    row 3: [stoneWall, stoneFloorWall]
    row 4: [stoneFloor, stoneFloor1, stoneFloor2, stoneFloor3]
    row 5: [stoneOutside, stoneOutside1, stoneOutside2, stoneOutside3]
    row 6: [brickWall, brickFloorWall1, brickFloorWall2]
    row 7: [brickFloor, brickFloor1, brickFloor2, brickFloor3]
    row 8: [brickOutside, brickOutside1, brickOutside2, brickOutside3]
    row 9: [bedrockWall,bedrockFloorWall]
    row 10: [bedrockFloor, bedrockFloor1, bedrockFloor2, bedrockFloor3]
    row 11: [bedrockOutside, bedrockOutside1, bedrockOutside2, bedrock Outside3]
    row 12: [redbrickWall, redbrickFloorWall, redbrickFloorWall1]
    row 13: [redrbrickFloor, redbrickFloor1, redbrickFloor2, redbrickFloor3]
    row 14: [redbrickOutside, redbrickOutside1, redbrickOutside2, redbrickOutside3]
    row 15: [boneWall, boneFloorWall]
    row 16: [boneFloor, boneFloor1, boneFloor2, boneFloor3]
    row 17: [boneOutside, boneOutside1, boneOutside2, boneOutside3]
- Tiles that are neither floor nor wall are outside blocks and use the xOutside tiles at random.
- Tiles that are Walls that have no xFloor tile below them are xWall or xWall1 tiles at random.
- Tiles that are Floors will randomly use xFloor tiles
- Floors 1-10 use dirt, 11-20 use stone, 21-30 use brick, 31-40 use bedrock, 41-50 use redbrick, and 51+ use bone.


Player atlas:
assets/icons/player_atlas.png

Expected layout:
- Cell size: 32x32 pixels per icon
- The game currently reads class icons from row 0:
  Col 0: Warrior
  Col 1: Ranger
  Col 2: Wizard
- Minimum recommended atlas size: 96x32
- Extra columns/rows are allowed and ignored by class rendering.


Enemy atlas:
assets/icons/enemy_atlas.png

Expected layout:
- Cell size: 32x32 pixels per icon
- Any grid size is valid (e.g. 64x64, 96x64, 160x96, etc.)
- Every 32x32 tile in the sheet is considered an enemy variant.
- Enemies pick a random tile on spawn from all available tiles.
- Tile order is row-major: left-to-right, then top-to-bottom.
- grid used by game:
    row 0: [goblinWarrior, goblinRanger, goblinWizard]
    row 1: [undeadWarrior, undeadRanger, undeadWizard]
    row 2: [beastRat, beastWolf, beastBat]

If either player/enemy atlas is missing or too small, the game falls back to simple colored rectangles.


NPC atlas:
assets/icons/npc_atlas.png

Expected layout:
- Cell size: 32x32 pixels per icon
- Any grid size is valid.
- Every 32x32 tile is considered a valid merchant NPC portrait.
- On merchant floors, the merchant icon is chosen randomly from all tiles in this atlas.
- If the atlas is missing or invalid, the merchant falls back to the default "$" marker.


Action atlas:
assets/icons/action_atlas.png

Expected layout:
- Cell size: 32x32 pixels per icon
- Row 0 is used for interactable state icons.
- Col 0: locked/inactive state icon
- Col 1: unlocked/active state icon
- Currently used for Merchant chamber door state.
- If missing/invalid, the game falls back to drawn primitive lock/arch markers.
- Grid used by the game:
    row 0: [woodDoorClosed, woodDoorOpen]
    row 1: [stoneDoorClosed, stoneDoorOpen]
    row 2: [chestClosed, chestOpen]
    row 3: [potClosed, potOpen]
    row 4: [buttonUp, buttonDown]
    row 5: [spikesDown, spikesUp]
- Wood Door used on floors that use dirt or redbrick tiles
- Stone Door used on floors that use stone, brick, bedrock, and bone tiles
- Chests spawn Closed, no more than 2 chests per floor, minimum 0. Opening a chest generates a random equipment and gold pickup
- Pots spawn Closed, no more than 10 pots per floor, minimum 0. Opening a pot generates a random consumable and gold pickup.
- buttons spawn UP, walking on button turns it into buttonDown. Each button is linked to a random door blocking a path (Excluding merchant door). Pressing the button opens that door.
- Spikes spawn Down, walking on spikes turns it into spikesUp and deals 20% MaxHP damage before armor calculation. Flying creatures are immune to spikes.


Walkables atlas:
assets/icons/walkables_atlas.png

Expected layout:
- Cell size: 32x32 pixels per icon
- Any grid size is valid; every tile is a variant.
- Variants are dotted randomly on floor tiles as walk-over decorations.
- Walkable decorations do NOT block player/enemy movement.
- No more than 20 walkables per floor


Solids atlas:
assets/icons/solids_atlas.png

Expected layout:
- Cell size: 32x32 pixels per icon
- Any grid size is valid; every tile is a variant.
- Variants are dotted randomly on floor tiles as blocking decorations.
- Solid decorations DO block player and enemy movement/pathing.
- No more than 5 solids per chamber
- Hallways cannot have Solids in them
- Game uses this grid:
    row 0: [boulder, rock, barrel, pot, logs]
    row 1: [closedCasket, brokenCasket, openCasket, closedCoffin, brokenCoffin, openCoffin]

Stairs:
assets/Icons/stairs.png

Expected layout:
- Single sprite used for the stairs to move to the next floor.



AUDIO
audio asset filepath:
asstes/audio/

melee: [26_sword_hit_1.wav, 26_sword_hit_2.wav, 26_sword_hit_3.wav]
magic: [04_Fire_explosion_04_medium.wav]
ranged: [36_Miss_Evade_02.wav]
beast attack: [08_bite_04.wav, 03_Claw_03.wav]
player damaged: [11_human_damage_1.wav, 11_human_damage_2.wav, 11_human_damage_3.wav]
enemy damaged: [21_orc_damage_1.wav, 21_orc_damage_2.wav, 21_orc_damage_3.wav]
death: [69_Enemy_death_01.wav]

mouse over: [001_hover_01.wav]
confirm: [013_Confirm_03.wav]
decline: [029_Decline_09.wav]
menu open: [092_Pause_04.wav]
menu close: [098_Unpause_04.wav]
equip item: [070_equip_10.wav]
unequip item: [071_unequip_01.wav]
use item: [051_use_item_01.wav]
denied: [033_Denited_03.wav]
Buy/Sell/Coin: [079_Buy_sell_01.wav]

door open: [05_door_open_1.mp3, 05_door_open_2.mp3]
door close: [06_door_close_1.mp3, 06_door_close_2.mp3]
chest open: [01_chest_open_1.wav, 01_chest_open_2.wav, 01_chest_open_3.wav, 01_chest_open_4.wav]
chest close: [02_chest_close_1.wav, 02_chest_close_2.wav, 02_chest_close_3.wav]


BGM: [Goblins_Den_(Regular).wav]