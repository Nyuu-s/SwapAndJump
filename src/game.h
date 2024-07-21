#pragma once

#include "input.h"
#include "SAJ_lib.h"
#include "gl_render_interface.h"


constexpr int WORLD_WIDTH = 320;
constexpr int WORLD_HEIGHT = 180;
constexpr int TILESIZE = 8;


//########################################################################
// Game Struct
//########################################################################
struct GameState
{
    bool initialized = false;
    IVec2 playerPos;
};

static GameState* gameState;

//########################################################################
// Game Functions (exposed)
//########################################################################
extern "C"
{
    EXPORT_FN void update_game(GameState* gameStateIn, RenderData* data, Input* inputIn);
};