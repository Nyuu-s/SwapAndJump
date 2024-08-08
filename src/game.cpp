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
  
    // Neighbouring Tiles        Top    Left      Right       Bottom  
    int neighbourOffsets[24] = { 0,-1,  -1, 0,     1, 0,       0, 1,   
    //                          Topleft Topright Bottomleft Bottomright
                                -1,-1,   1,-1,    -1, 1,       1, 1,
    //                           Top2   Left2     Right2      Bottom2
                                 0,-2,  -2, 0,     2, 0,       0, 2}; 

    unsigned char cardinalflags = 0;
    unsigned char diagflags = 0;
    int neighbourCount = 0;
    int slot = 0;
    Tile* tile = get_tile(x, y);
    if(!tile)
    {
        return 0;
    }
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
        tile->neighbourMask = 20;
        return 20;
    }
    if(neighbourCount == 7 && slot >= 4 )
    {
        tile->neighbourMask = 16 + (slot - 4);
        return 16 + (slot - 4);
    }
    tile->neighbourMask = cardinalflags & 0xF;
    return cardinalflags & 0xF;
}


void simulate()
{
    gameState->player.prevPos = gameState->player.pos;
    if(is_down(MOVE_LEFT))
    {
        gameState->player.pos.x -= 1;
    }
    if(is_down(MOVE_UP))
    {
        gameState->player.pos.y -= 1;
    }
       if(is_down(MOVE_DOWN))
    {
        gameState->player.pos.y += 1;
    }
       if(is_down(MOVE_RIGHT))
    {
        gameState->player.pos.x += 1;
    }
    bool shouldUpdateTiles = false;
    if(is_down(MOUSE_LEFT))
    {
        IVec2 mousePos = input->mousePositionWorld;
        
        Tile* tile = get_tile(mousePos);
        if(tile)
        {
            tile->isVisible = true;
            shouldUpdateTiles = true;
        }
    }
    if(is_down(MOUSE_RIGHT))
    {
        IVec2 mousePos = input->mousePositionWorld;
        Tile* tile = get_tile(mousePos);
        if(tile)
        {
            tile->isVisible = false;
            shouldUpdateTiles = true;
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
            if(shouldUpdateTiles)
            {
                get_bitmask_index(j, i);
            }
        }
    }
}
//########################################################################
// Exposed Game Functions
//########################################################################
EXPORT_FN void update_game(GameState* gameStateIn, RenderData* data, Input* inputIn, double deltatime)
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

    // update game loop dt
    {
        
        gameState->dtAccumulator += deltatime;
        //TODO decouple render dt and logic dt in order to use variable deltatime instead of a fixed one
        //replace delay by dt

        while (gameState->dtAccumulator >= UPDATE_DELAY)
        {
            gameState->dtAccumulator -= UPDATE_DELAY;
            simulate();

            //get relative mouse position because more frames than simulations
            input->relativeMousePosition = input->mousePosition - input->prevMousePosition;
            input->prevMousePosition = input->mousePosition;

            // Reset inputs inorder to get new one
            {
                for (size_t keyCode = 0; keyCode < KEYCODE_COUNT; keyCode++)
                {
                    input->keys[keyCode].isPressed = false;
                    input->keys[keyCode].isReleased = false;
                    input->keys[keyCode].halfTransitionCount = 0;
                }
                
            }
        }
        
    }

    double interpolatedDT = (double)(gameState->dtAccumulator / UPDATE_DELAY);

    {
        Player& player = gameState->player;
        IVec2 playerPos = lerp(player.prevPos, player.pos, interpolatedDT);
        draw_sprite(SPRITE_DICE, playerPos);
      
    }


    //DRAW TILES 
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


        SpriteTransform transform = {};
        Vec2 fpos =   Vec2{j * (float)TILESIZE + (float)TILESIZE/2.0f, i * (float)TILESIZE + (float)TILESIZE/2.0f} - Vec2{TILESIZE, TILESIZE} / 2.0;
        transform.pos = {(int) fpos.x, (int)fpos.y};
        transform.size = {TILESIZE, TILESIZE};
        transform.spriteSize = {TILESIZE, TILESIZE};
        transform.spriteOffset = {initialSpriteOffset.x + (tile->neighbourMask % 4) * TILESIZE, initialSpriteOffset.y + (tile->neighbourMask/4) * TILESIZE};
     
        draw_quad(transform);
     }
     
    }
    
}