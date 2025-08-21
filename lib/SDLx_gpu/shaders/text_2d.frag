Texture2D<float4> PaletteTexture : register(t0, space2);
SamplerState PaletteSampler : register(s0, space2);

struct Input
{
    float2 Texcoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
};

float4 main(Input input) : SV_Target
{
    float4 color = PaletteTexture.Sample(PaletteSampler, input.Texcoord) * input.Color;
    if (color.a < 0.0001f)
    {
        clip(-1);
    }
    return color;
}