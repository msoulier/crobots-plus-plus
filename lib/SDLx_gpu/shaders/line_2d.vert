#include "shader.hlsl"

cbuffer UniformOrtho : register(b0, space1)
{
    float4x4 Ortho : packoffset(c0);
};

struct Input
{
    float2 Position : TEXCOORD0;
    uint Color : TEXCOORD1;
};

struct Output
{
    float4 Position : SV_POSITION;
    float4 Color : TEXCOORD0;
};

Output main(Input input)
{
    Output output;
    output.Color = GetColorFromU32(input.Color);
    output.Position = mul(Ortho, float4(input.Position, 0.0f, 1.0f));
    return output;
}