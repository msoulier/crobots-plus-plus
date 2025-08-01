#version 450

layout(location = 0) in vec3 inLocalPosition;
layout(location = 1) in vec3 inWorldPosition;
layout(location = 2) in uint inColor;
layout(location = 3) in uint inLifetime;
layout(location = 0) out flat uint outColor;
/* TODO: outLifetime for fade to invisible */
layout(set = 1, binding = 0) uniform uniformViewProjMatrix
{
    mat4 viewProjMatrix;
};

void main()
{
    outColor = inColor;
    gl_Position = viewProjMatrix * vec4(inWorldPosition + inLocalPosition, 1.0f);
}