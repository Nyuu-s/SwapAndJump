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

IVec2 get_grid_pos(IVec2 worldpos)
{
    return {(worldpos.x / TILESIZE) , (worldpos.y / TILESIZE)  };
}

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
    IVec2 gridpos = get_grid_pos(worldPos);
    return get_tile(gridpos.x, gridpos.y);
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

IRect get_player_aabb()
{
    return
    {
        gameState->player.pos.x - 4,
        gameState->player.pos.y -8,
        8,
        16
    };
}

IVec2 get_tile_pos_from_grid(int x, int y)
{
    IVec2 pos =   IVec2{x * TILESIZE, y * TILESIZE};
    
    return {(x * TILESIZE), y * TILESIZE};

}
IRect get_tile_rect(int x, int y)
{
    return {get_tile_pos_from_grid(x, y), 8, 8};
}

void simulate()
{
    float dt = UPDATE_DELAY;
    Player& player = gameState->player;
    player.prevPos = player.pos;
    

    static Vec2 remainder = {};
    static bool grounded = false;

    constexpr float runSpeed = 2.0f;
    constexpr float runAcceleration = 10.0f;
    constexpr float runReduce = 22.0F;
    constexpr float flyReduce = 12.0f;
    constexpr float gravity = 13.0f;
    constexpr float fallSpeed = 3.6f;
    constexpr float jumpSpeed = -4.0f;
    //Jump
    if(just_pressed(JUMP) && grounded)
    {
        player.speed.y = jumpSpeed;
        grounded = false;
    }
    if(is_down(MOVE_LEFT))
    {
        float mult = 1.0f;
        if(player.speed.x > 0.0f)
        {
            mult = 3.0;
        }
        player.speed.x = approach(player.speed.x, -runSpeed, runAcceleration * mult * dt);
    }
    if(is_down(MOVE_RIGHT))
    {
        float mult = 1.0f;
        if(player.speed.x < 0.0f)
        {
            mult = 3.0;
        }
        player.speed.x = approach(player.speed.x, runSpeed, runAcceleration * mult * dt);
    }
    //Friction
    if(!is_down(MOVE_LEFT) && !is_down(MOVE_RIGHT))
    {
        if(grounded)
        {
            player.speed.x = approach(player.speed.x, 0, runReduce * dt);
        }
        else
        {
            player.speed.x = approach(player.speed.x, 0, flyReduce * dt);
        }
    }
    //Gravity
    player.speed.y = approach(player.speed.y, fallSpeed, gravity * dt);

    if(is_down(MOVE_UP))
    {
        player.pos = {};
    }
    
    // MOVE X
    {
        IRect playerRect = get_player_aabb();

        remainder.x += player.speed.x;
        int moveX = round(remainder.x);

        if(moveX != 0)
        {
            remainder.x -= moveX;
            int moveSign = sign(moveX); // return 1 or -1
            bool didCollide = false;

            // move the player in Y until colliison or moveY is exhausted
            //lambda function
            auto movePlayerX = [&]
            {
                while (moveX)   
                {
                    playerRect.pos.x += moveSign;

                    IVec2 playerGridPos = get_grid_pos(player.pos);
                    for (size_t x = playerGridPos.x - 1; x <= playerGridPos.x + 1; x++)
                    {
                        for (size_t y = playerGridPos.y - 2; y <= playerGridPos.y + 2; y++)
                        {
                            Tile* tile = get_tile(x, y);
                            if(!tile || !tile->isVisible)
                            {
                                continue;
                            }

                            IRect tileRect = get_tile_rect(x, y);
                            if(rect_collision(playerRect, tileRect))
                            {
                                player.speed.x = 0;
                                return;
                            }
                        }
                        
                    }
                    player.pos.x += moveSign;
                    moveX -= moveSign;   
                }
            };
            movePlayerX();
        }
    }
    // MOVE Y
    {
        IRect playerRect = get_player_aabb();

        remainder.y += player.speed.y;
        int moveY = round(remainder.y);

        if(moveY != 0)
        {
            remainder.y -= moveY;
            int moveSign = sign(moveY); // return 1 or -1
            bool didCollide = false;

            // move the player in Y until colliison or moveY is exhausted
            //lambda function
            auto movePlayerY = [&]
            {
                while (moveY)   
                {
                    playerRect.pos.y += moveSign;

                    IVec2 playerGridPos = get_grid_pos(player.pos);
                    for (size_t x = playerGridPos.x - 1; x <= playerGridPos.x + 1; x++)
                    {
                        for (size_t y = playerGridPos.y - 2; y <= playerGridPos.y + 2; y++)
                        {
                            Tile* tile = get_tile(x, y);
                            if(!tile || !tile->isVisible)
                            {
                                continue;
                            }

                            IRect tileRect = get_tile_rect(x, y);
                            if(rect_collision(playerRect, tileRect))
                            {
                                if(player.speed.y > 0.0f)
                                {
                                    grounded = true;
                                }
                                player.speed.y = 0;
                                return;
                            }
                        }
                        
                    }
                    player.pos.y += moveSign;
                    moveY -= moveSign;   
                }
            };
            movePlayerY();
        }
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
            gameState->keyMappings[JUMP].keys.add(KEY_SPACE);
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
        draw_sprite(SPRITE_CHARACHTER, lerp(player.prevPos, player.pos, (float)interpolatedDT));
        // draw_sprite(SPRITE_CHARACHTER, player.pos);
      
    }


    //DRAW TILES 
    IVec2 initialSpriteOffset = {7*16, 0};
    for (int i = 0; i < WORLD_GRID.y; i++)
    {
     for (int j = 0; j < WORLD_GRID.x; j++)
     {
        Tile* tile = get_tile(j, i);
        // tile->isVisible = true;
        // tile->neighbourMask = 20;
        if(j == WORLD_GRID.x/2 && i == WORLD_GRID.y / 2)
        {
            tile->isVisible = true;
            tile->neighbourMask = 20;
        }

        if(!tile || !tile->isVisible)
        {
            continue;
        }

        SpriteTransform transform = {};
        Vec2 fpos =   Vec2{j * (float)TILESIZE, i * (float)TILESIZE};
        transform.pos = {(int) fpos.x, (int)fpos.y};
        transform.size = {TILESIZE, TILESIZE};
        transform.spriteSize = {TILESIZE, TILESIZE};
        transform.spriteOffset = {initialSpriteOffset.x + (tile->neighbourMask % 4) * TILESIZE, initialSpriteOffset.y + (tile->neighbourMask/4) * TILESIZE};
     
        draw_quad(transform);
     }
     
    }
    
}