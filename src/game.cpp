#include "game.h"
#include "assets.h"


EXPORT_FN void update_game(GameState* gameStateIn, RenderData* data, Input* inputIn)
{
    if(renderData != data)
    {
        gameState = gameStateIn;
        renderData = data;
        input = inputIn;
    }

    if(!gameState->initialized)
    {
        renderData->gameCamera.dimensions = {WORLD_WIDTH, WORLD_HEIGHT};
        gameState->initialized = true;
    }

        // renderData->gameCamera.position.x = 160;
        // renderData->gameCamera.position.y = 90;
   
    draw_sprite(SPRITE_DICE, gameState->playerPos);

    if(key_is_down(KEY_Q))
    {
        gameState->playerPos.x -= 1;
    }
    if(key_is_down(KEY_Z))
    {
        gameState->playerPos.y -= 1;
    }
       if(key_is_down(KEY_S))
    {
        gameState->playerPos.y += 1;
    }
       if(key_is_down(KEY_D))
    {
        gameState->playerPos.x += 1;
    }
    
}