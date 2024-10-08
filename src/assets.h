#pragma once

#include "SAJ_lib.h"

enum SpriteID
{
    SPRITE_DICE,
    SPRITE_WHITE,
    SPRITE_CHARACHTER,
    SPRITE_SOLID_01,
    SPRITE_SOLID_02,



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
            sprite.spriteOffset = {16, 0};
            sprite.spriteSize = {16, 16};
            break;   
        }
        case SPRITE_WHITE:
        {
            sprite.spriteOffset = {0, 0};
            sprite.spriteSize = {1, 1};
            break;
        }
        case SPRITE_SOLID_01:
        {
            sprite.spriteOffset = {83, 80};
            sprite.spriteSize = {28, 18};
            break;
        }
        case SPRITE_SOLID_02:
        {

            sprite.spriteOffset = {64, 80};
            sprite.spriteSize = {16, 13};
            break;
        }
        case SPRITE_CHARACHTER:
        {
            sprite.spriteOffset = {176, 0};
            sprite.spriteSize = {17, 20};
            break;
        }
    }

    return sprite;
}