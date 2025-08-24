#include "shader.hlsl"

cbuffer UniformViewProj : register(b0, space1)
{
    float4x4 ViewProj : packoffset(c0);
};

cbuffer UniformInstance : register(b1, space1)
{
    float4x4 Model : packoffset(c0);
    uint Color : packoffset(c4.x);
};

struct Input
{
    float2 Position : TEXCOORD0;
    float2 Texcoord : TEXCOORD1;
};

struct Output
{
    float4 Position : SV_POSITION;
    float2 Texcoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
};

Output main(Input input)
{
    Output output;
    output.Color = GetColorFromU32(Color);
    output.Texcoord = input.Texcoord;
    output.Position = mul(ViewProj, mul(Model, float4(input.Position, 0.0f, 1.0f)));
    return output;
}