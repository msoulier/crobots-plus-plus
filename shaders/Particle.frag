#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in flat uint inColor;

void main()
{
    outColor.r = (inColor >> 0) & 0xFFu;
    outColor.g = (inColor >> 8) & 0xFFu;
    outColor.b = (inColor >> 16) & 0xFFu;
    outColor.a = (inColor >> 24) & 0xFFu;
}