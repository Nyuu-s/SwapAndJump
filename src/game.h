#pragma once

#include "input.h"
#include "SAJ_lib.h"
#include "gl_render_interface.h"


constexpr int WORLD_WIDTH = 320;
constexpr int WORLD_HEIGHT = 180;
constexpr int TILESIZE = 8;
constexpr IVec2 WORLD_GRID = {WORLD_WIDTH/TILESIZE, WORLD_HEIGHT/TILESIZE};
constexpr int UPDATE_PER_SEC = 60;
constexpr double UPDATE_DELAY = 1.0 / UPDATE_PER_SEC;


enum GameInputType
{
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    JUMP,
    MOUSE_LEFT,
    MOUSE_RIGHT,

    GAME_INPUT_COUNT
};

struct Tile
{
    int neighbourMask;
    bool isVisible;
};

//########################################################################
// Game Struct
//########################################################################
struct KeyMapping
{
  ArrayDef(KeyCodeBinding, 3) keys;  
};

struct Player 
{
    IVec2 pos;
    IVec2 prevPos;
    Vec2 speed;
    Vec2 solidSpeed;
};

struct Solid {
    SpriteID spriteID;
    IVec2 pos;
    IVec2 prevPos;
    Vec2 remainder;
    Vec2 speed;
    int keyframeidx;
    ArrayDef(IVec2, 2) keyframes;
};

struct GameState
{
    bool initialized = false;
    double dtAccumulator = 0.0;
    Player player;

    KeyMapping keyMappings[GAME_INPUT_COUNT];
    Tile worldGrid[WORLD_GRID.x][WORLD_GRID.y];
    ArrayDef(Solid, 20) solids;
};

static GameState* gameState;



//########################################################################
// Game Functions (exposed)
//########################################################################
extern "C"
{
    EXPORT_FN void update_game(GameState* gameStateIn, RenderData* data, Input* inputIn, double deltatime);
};