#pragma once

#include "SAJ_lib.h"

enum SpriteID
{
    SPRITE_DICE,

    SPRITE_COUNT
};

struct Sprite
{
    IVec2 spriteOffset;
    IVec2 spriteSize;
};

Sprite get_sprite(SpriteID SpriteID)
{   
    Sprite sprite = {};
    switch(SpriteID)
    {
        case SPRITE_DICE:
        {
            sprite.spriteOffset = {0, 0};
            sprite.spriteSize = {16, 16};
            
        }
    }

    return sprite;
}