#pragma once

#include "input.h"
#include "SAJ_lib.h"
#include "gl_render_interface.h"


constexpr int WORLD_WIDTH = 320;
constexpr int WORLD_HEIGHT = 180;
constexpr int TILESIZE = 8;
constexpr IVec2 WORLD_GRID = {WORLD_WIDTH/TILESIZE,WORLD_HEIGHT/TILESIZE};
// constexpr Mat3 TileBitMaskKernel = {
//     1, 2, 4,
//     8, 0, 16,
//     32, 64, 128
// };
constexpr Mat3 TileBitMaskKernel = {
    0, 1, 0,
    2, 0, 4,
    0, 8, 0
};

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


struct GameState
{
    bool initialized = false;
    IVec2 playerPos;
    KeyMapping keyMappings[GAME_INPUT_COUNT];
    Tile worldGrid[WORLD_GRID.x][WORLD_GRID.y];
};

static GameState* gameState;


Tile* get_tile(int x, int y)
{
    Tile* tile = nullptr;
    if(x >= 0 && x<WORLD_GRID.x && y >=0 && y < WORLD_GRID.y)
    {
        tile = &gameState->worldGrid[x][y];
    }

    return tile;
}

Tile* get_tile(IVec2 worldPos)
{
    return get_tile(worldPos.x / TILESIZE, worldPos.y / TILESIZE);
}
//########################################################################
// Game Functions (exposed)
//########################################################################
extern "C"
{
    EXPORT_FN void update_game(GameState* gameStateIn, RenderData* data, Input* inputIn);
};