#pragma once

#include "assets.h"
#include "SAJ_lib.h"

constexpr int MAX_SPRITE_TRANSFORMS = 1000;

struct OrthoCamera2D
{
    float zoom = 1.0f;
    Vec2 dimensions;
    Vec2 position;    
};

struct SpriteTransform
{
    IVec2 spriteOffset;
    IVec2 spriteSize;
    IVec2 pos;
    IVec2 size;
};

struct RenderData
{
    OrthoCamera2D gameCamera;
    OrthoCamera2D uiCamera;

    int sTransformCount;
    SpriteTransform transforms[MAX_SPRITE_TRANSFORMS];
};


static RenderData* renderData;

IVec2 screen_to_world(IVec2 screenPos)
{
    OrthoCamera2D camera = renderData->gameCamera;
    int xPos = (float)screenPos.x / input->ScreenSize.x * camera.dimensions.x;
    xPos += -camera.dimensions.x / 2.0f + camera.position.x;

    int yPos = (float)screenPos.y / input->ScreenSize.y * camera.position.y;
    yPos += camera.dimensions.y / 2.0f + camera.position.y;

    return {xPos, yPos};
}

void draw_sprite(SpriteID spriteID, Vec2 pos)
{
    Sprite sprite = get_sprite(spriteID);

    SpriteTransform sTransform = {};
    sTransform.pos = {(int)pos.x, (int)pos.y};
    sTransform.spriteOffset = sprite.spriteOffset;
    sTransform.spriteSize = sprite.spriteSize;
    sTransform.size = sprite.spriteSize;
    renderData->transforms[renderData->sTransformCount++] = sTransform;
}

void draw_sprite(SpriteID spriteID, IVec2 pos)
{
    draw_sprite(spriteID, vec_2(pos));
}