#version 450

layout(location = 0) in uint inVertex;
layout(location = 0) out flat vec3 outNormal;
layout(location = 1) out vec2 outTexcoord;

vec3 getPosition(uint vertex)
{
    vec3 magnitude;
    vec3 direction;
    magnitude.x = float((vertex >>  0) & 0x3Fu);
    magnitude.y = float((vertex >>  6) & 0x1u);
    magnitude.z = float((vertex >>  7) & 0x3Fu);
    direction.x = float((vertex >> 13) & 0x1u);
    direction.y = float((vertex >> 14) & 0x3Fu);
    direction.z = float((vertex >> 20) & 0x1u);
    return (1.0f - 2.0f * direction) * magnitude;
}

const vec3 Normals[6] = vec3[6]
(
    vec3(-1.0f, 0.0f, 0.0f ),
    vec3( 1.0f, 0.0f, 0.0f ),
    vec3( 0.0f,-1.0f, 0.0f ),
    vec3( 0.0f, 1.0f, 0.0f ),
    vec3( 0.0f, 0.0f,-1.0f ),
    vec3( 0.0f, 0.0f, 1.0f )
);

vec3 getNormal(uint vertex)
{
    return Normals[(vertex >> 21) & 0x7u];
}

vec2 getTexcoord(uint vertex)
{
    return vec2(float((vertex >> 24) & 0xFFu) / 256.0f, 0.5f);
}

void main()
{
    vec3 position = getPosition(inVertex);
    outNormal = getNormal(inVertex);
    outTexcoord = getTexcoord(inVertex);
}