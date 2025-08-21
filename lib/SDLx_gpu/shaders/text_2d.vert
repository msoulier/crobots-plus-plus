#include "shader.hlsl"

cbuffer UniformOrtho : register(b0, space1)
{
    float4x4 Ortho : packoffset(c0);
};

cbuffer UniformInstance : register(b1, space1)
{
    float2 Position;
    uint Color;
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
    output.Position = mul(Ortho, float4(input.Position + Position, 0.0f, 1.0f));
    return output;
}