#include "game.h"
#include "assets.h"
//########################################################################
// Internal Game Functions
//########################################################################
bool just_pressed(GameInputType type)
{
    KeyMapping mapping = gameState->keyMappings[type];
    for (size_t i = 0; i < mapping.keys.count; i++)
    {
        if (input->keys[mapping.keys.elements[i]].isPressed)
        {
            return true;
        }   
    }
    return false;
}

bool is_down(GameInputType type)
{
    KeyMapping mapping = gameState->keyMappings[type];
    for (size_t i = 0; i < mapping.keys.count; i++)
    {
        if (input->keys[mapping.keys.elements[i]].isDown)
        {
            return true;
        }   
    }
    return false;
}

//########################################################################
// Exposed Game Functions
//########################################################################
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
        gameState->initialized = true;
        
        renderData->gameCamera.dimensions = {WORLD_WIDTH, WORLD_HEIGHT};
        renderData->gameCamera.position.x = 160;
        renderData->gameCamera.position.y = -90;

     
        {
            gameState->keyMappings[MOVE_UP].keys.add(KEY_Z);
            gameState->keyMappings[MOVE_UP].keys.add(KEY_ARROW_UP);
            gameState->keyMappings[MOVE_LEFT].keys.add(KEY_Q);
            gameState->keyMappings[MOVE_LEFT].keys.add(KEY_ARROW_LEFT);
            gameState->keyMappings[MOVE_RIGHT].keys.add(KEY_D);
            gameState->keyMappings[MOVE_RIGHT].keys.add(KEY_ARROW_RIGHT);
            gameState->keyMappings[MOVE_DOWN].keys.add(KEY_S);
            gameState->keyMappings[MOVE_DOWN].keys.add(KEY_ARROW_DOWN);

            gameState->keyMappings[MOUSE_LEFT].keys.add(KEY_MOUSE_LEFT);
            gameState->keyMappings[MOUSE_RIGHT].keys.add(KEY_MOUSE_RIGHT);
        } 

   
    }

    draw_sprite(SPRITE_DICE, gameState->playerPos);

    if(is_down(MOVE_LEFT))
    {
        gameState->playerPos.x -= 1;
    }
    if(is_down(MOVE_UP))
    {
        gameState->playerPos.y -= 1;
    }
       if(is_down(MOVE_DOWN))
    {
        gameState->playerPos.y += 1;
    }
       if(is_down(MOVE_RIGHT))
    {
        gameState->playerPos.x += 1;
    }
    if(is_down(MOUSE_LEFT))
    {
        IVec2 mousePos = input->mousePositionWorld;
       
        Tile* tile = get_tile(mousePos);
        if(tile)
        {
            tile->isVisible = true;
        }
    }
    if(is_down(MOUSE_RIGHT))
    {
        IVec2 mousePos = input->mousePositionWorld;
        Tile* tile = get_tile(mousePos);
        if(tile)
        {
            tile->isVisible = false;
        }
    }

    for (size_t i = 0; i < WORLD_GRID.y; i++)
    {
     for (size_t j = 0; j < WORLD_GRID.x; j++)
     {
        Tile* tile = get_tile(j, i);
        if(!tile->isVisible)
        {
            continue;
        }

        Vec2 tilepos = {j * (float)TILESIZE + (float)TILESIZE/2.0f, i * (float)TILESIZE + (float)TILESIZE/2.0f};
        draw_quad(tilepos, {8, 8});
     }
     
    }
    
}