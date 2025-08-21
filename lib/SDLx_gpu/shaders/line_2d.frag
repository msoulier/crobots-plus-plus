struct Input
{
    float4 Color : TEXCOORD0;
};

float4 main(Input input) : SV_Target
{
    return float4(input.Color);
}