#pragma once

#include "assets.h"
#include "SAJ_lib.h"


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

    
    ArrayDef(SpriteTransform, 1000) transforms;
    
};


static RenderData* renderData;

IVec2 screen_to_world(IVec2 screenPos)
{
    OrthoCamera2D camera = renderData->gameCamera;

    int xPos = (float)screenPos.x / (float)input->ScreenSize.x * camera.dimensions.x;
    xPos += -camera.dimensions.x / 2.0f + camera.position.x;

    int yPos = (float)screenPos.y / (float)input->ScreenSize.y * camera.dimensions.y;
    yPos += camera.dimensions.y / 2.0f + camera.position.y;

    return {xPos, yPos};

    
}
void draw_quad(SpriteTransform transform)
{
    renderData->transforms.add(transform);
}
void draw_quad(Vec2 pos, Vec2 size)
{
    SpriteTransform transform = {};
    Vec2 fpos = pos - size  / 2.0f;  
    transform.pos =  {(int)fpos.x, (int)fpos.y};
    transform.size = {(int)size.x, (int) size.y};
    transform.spriteOffset = {0, 0};
    transform.spriteSize = {1, 1}; // index to white

    renderData->transforms.add(transform);
}
void draw_sprite(SpriteID spriteID, Vec2 pos)
{
    Sprite sprite = get_sprite(spriteID);

    SpriteTransform sTransform = {};
    sTransform.pos = IVec2{(int)pos.x, (int)pos.y} - sprite.spriteSize / 2.0f;
    sTransform.spriteOffset = sprite.spriteOffset;
    sTransform.spriteSize = sprite.spriteSize;
    sTransform.size = sprite.spriteSize;
    renderData->transforms.add(sTransform);
}

void draw_sprite(SpriteID spriteID, IVec2 pos)
{
    draw_sprite(spriteID, vec_2(pos));
}