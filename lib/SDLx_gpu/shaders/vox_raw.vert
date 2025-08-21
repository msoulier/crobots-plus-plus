#include "shader.hlsl"

cbuffer UniformViewProj : register(b0, space1)
{
    float4x4 ViewProj : packoffset(c0);
};

cbuffer UniformModel : register(b1, space1)
{
    float4x4 Model : packoffset(c0);
};

struct Input
{
    float3 Position : TEXCOORD0;
    float3 Instance : TEXCOORD1;
    uint Color : TEXCOORD2;
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
    output.Position = mul(ViewProj, mul(Model, float4(input.Instance + input.Position, 1.0f)));
    return output;
}