#pragma once

#include "assets.h"
#include "SAJ_lib.h"

constexpr int MAX_SPRITE_TRANSFORMS = 1000;

struct SpriteTransform
{
    IVec2 spriteOffset;
    IVec2 spriteSize;
    IVec2 pos;
    IVec2 size;
};

struct RenderData
{
    int sTransformCount;
    SpriteTransform transforms[MAX_SPRITE_TRANSFORMS];
};


static RenderData renderData;

void draw_sprite(SpriteID spriteID, Vec2 pos, Vec2 size)
{
    Sprite sprite = get_sprite(spriteID);

    SpriteTransform sTransform = {};
    sTransform.pos = {(int)pos.x, (int)pos.y};
    sTransform.size = {(int)size.x, (int)size.y};
    sTransform.spriteOffset = sprite.spriteOffset;
    sTransform.spriteSize = sprite.spriteSize;

    renderData.transforms[renderData.sTransformCount++] = sTransform;
}