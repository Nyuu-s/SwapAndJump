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

        
    // Neighbouring Tiles        Top    Left      Right       Bottom  
    int neighbourOffsets[24] = { 0,-1,  -1, 0,     1, 0,       0, 1,   
    //                          Topleft Topright Bottomleft Bottomright
                                -1,-1,   1,-1,    -1, 1,       1, 1,
    //                           Top2   Left2     Right2      Bottom2
                                 0,-2,  -2, 0,     2, 0,       0, 2};

    Tile* left_tile = get_tile(x-1, y);
    if(left_tile)
    {
        left = left_tile->isVisible;
    }

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


    unsigned char cardinalflags = 0;
    unsigned char diagflags = 0;
    int neighbourCount = 0;
    int slot = 0;
    for(int n = 0; n < 8; n++)
    {
   
        Tile* neighbour  = get_tile(x + neighbourOffsets[n * 2], y + neighbourOffsets[n * 2 + 1]);
        if(!neighbour || neighbour->isVisible)
        {
            cardinalflags |= 1 << n;  
            neighbourCount++;
        }
        else
        {
            slot = n;
        }
    }

    if (cardinalflags == 255)
    {
        return 20;
    }
    if(neighbourCount == 7 && slot >= 4 )
    {
        return 16 + (slot - 4);
    }

    return cardinalflags & 0xF;
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

    IVec2 initialSpriteOffset = {7*16, 0};
    for (size_t i = 0; i < WORLD_GRID.y; i++)
    {
     for (size_t j = 0; j < WORLD_GRID.x; j++)
     {
        Tile* tile = get_tile(j, i);
        if(!tile->isVisible)
        {
            continue;
        }

        
        int index =  get_bitmask_index(j, i);

        SpriteTransform transform = {};
        Vec2 fpos =   Vec2{j * (float)TILESIZE + (float)TILESIZE/2.0f, i * (float)TILESIZE + (float)TILESIZE/2.0f} - Vec2{TILESIZE, TILESIZE} / 2.0;
        transform.pos = {(int) fpos.x, (int)fpos.y};
        transform.size = {TILESIZE, TILESIZE};
        transform.spriteSize = {TILESIZE, TILESIZE};
        transform.spriteOffset = {initialSpriteOffset.x + (index % 4) * TILESIZE, initialSpriteOffset.y + (index/4) * TILESIZE};
     
        draw_quad(transform);
     }
     
    }
    
}