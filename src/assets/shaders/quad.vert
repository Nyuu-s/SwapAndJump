#version 430 core
//Structs
struct SpriteTransform
{
    ivec2 spriteOffset;
    ivec2 spriteSize;
    ivec2 pos;
    ivec2 size;
};




//Input
layout (std430, binding = 0) buffer TransformSBO
{
    SpriteTransform transforms[];
};
//Output 
layout (location = 0) out vec2 textureCoordsOut;
uniform vec2 screenSize;
uniform mat4 orthoProjection;

void main()
{
    SpriteTransform sTransform = transforms[gl_InstanceID];

  vec2 vertices[6] =
  {
    sTransform.pos,                                        // Top Left
    vec2(sTransform.pos + vec2(0.0, sTransform.size.y)),    // Bottom Left
    vec2(sTransform.pos + vec2(sTransform.size.x, 0.0)),    // Top Right
    vec2(sTransform.pos + vec2(sTransform.size.x, 0.0)),    // Top Right
    vec2(sTransform.pos + vec2(0.0, sTransform.size.y)),    // Bottom Left
    sTransform.pos + sTransform.size                        // Bottom Right
  };

    float top = sTransform.spriteOffset.y;
    float bottom = sTransform.spriteOffset.y + sTransform.spriteSize.y;
    float right = sTransform.spriteOffset.x + sTransform.spriteSize.x;
    float left = sTransform.spriteOffset.x;

    vec2 textureCoords[6] = 
    {
        vec2(left, top),
        vec2(left, bottom),
        vec2(right, top),
        vec2(right, top),
        vec2(left, bottom),
        vec2(right, bottom),
    };
    //Normilize position to screensize
    {
        vec2 vertexPos = vertices[gl_VertexID];
        gl_Position = orthoProjection * vec4(vertexPos, 0.0, 1.0);

    }

    textureCoordsOut = textureCoords[gl_VertexID]; 
}