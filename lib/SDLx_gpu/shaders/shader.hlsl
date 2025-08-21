#ifndef SHADER_HLSL
#define SHADER_HLSL

float4 GetColorFromU32(uint color)
{
    float4 outColor;
    outColor.r = float((color >> 24) & 0xFFu) / 255.0f;
    outColor.g = float((color >> 16) & 0xFFu) / 255.0f;
    outColor.b = float((color >> 8) & 0xFFu) / 255.0f;
    outColor.a = float((color >> 0) & 0xFFu) / 255.0f;
    return outColor;
}

#endif