#include "shader.hlsl"

cbuffer UniformViewProj : register(b0, space1)
{
    float4x4 ViewProj : packoffset(c0);
};

struct Input
{
    float3 Position : TEXCOORD0;
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
    output.Position = mul(ViewProj, float4(input.Position, 1.0f));
    return output;
}