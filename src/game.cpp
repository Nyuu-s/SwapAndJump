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

int get_bitmask_index(int x, int y)
{
    bool left =1;
    bool right = 1;
    bool top = 1;
    bool bottom = 1;
    bool top_left = 1;
    bool top_right = 1;
    bool bottom_left = 1;
    bool bottom_right = 1;

        




    Tile* right_tile = get_tile(x+1, y);
    if(right_tile)
    {
        right = right_tile->isVisible;
    }
    Tile* top_tile =   get_tile(x, y-1);
    if(top_tile)
    {
        top = top_tile->isVisible;
    }
    Tile* bottom_tile = get_tile(x, y+1);
    if(bottom_tile)
    {
        bottom = bottom_tile->isVisible;
    }
    Tile* bottom_left_tile = get_tile(x-1, y+1);
    if(bottom_tile)
    {
        bottom = bottom_tile->isVisible;
    }
    Tile* bottom_right_tile = get_tile(x+1, y+1);
    if(bottom_tile)
    {
        bottom = bottom_tile->isVisible;
    }
    int cardinal =  TileBitMaskKernel.bx * top 
    + TileBitMaskKernel.ay * left 
    + TileBitMaskKernel.cy * right 
    + TileBitMaskKernel.bz * bottom;


    unsigned char flags = 0xFF;

    
    for (int i = -1; i < 2; i++)
    {
        for (int j = -1; j < 2 ; j++)
        {
           
            Tile* neighbour  = get_tile(x+j, y+i);
            if(neighbour)
            {
               flags = flags << 1;
               flags |= neighbour->isVisible;
               
            }
        }
        
    }
    flags = flags & ~16;

    return cardinal;
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
        SpriteTransform transform = {};
        Vec2 fpos =  tilepos - Vec2{TILESIZE, TILESIZE} / 2.0;
        transform.pos = {(int) fpos.x, (int)fpos.y};
        transform.size = {TILESIZE, TILESIZE};
        transform.spriteSize = {TILESIZE, TILESIZE};
        int index =  get_bitmask_index(j, i);

        transform.spriteOffset = {7*16 + (index % 4) * TILESIZE, (index/4) * TILESIZE};
    
        draw_quad(transform);
     }
     
    }
    
}