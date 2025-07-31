#version 450

layout(location = 0) in uvec2 inVertex;
layout(location = 0) out flat vec3 outNormal;
layout(location = 1) out vec2 outTexcoord;
layout(set = 1, binding = 0) uniform uniformViewProjMatrix
{
    mat4 viewProjMatrix;
};
layout(set = 1, binding = 1) uniform uniformModelMatrix
{
    mat4 modelMatrix;
};

vec3 getPosition(uvec2 vertex)
{
    vec3 magnitude;
    vec3 direction;
    magnitude.x = float((vertex.x >>  0) & 0xFFu);
    direction.x = float((vertex.x >>  8) & 0x1u);
    magnitude.y = float((vertex.x >>  9) & 0xFFu);
    direction.y = float((vertex.x >> 17) & 0x1u);
    magnitude.z = float((vertex.x >> 18) & 0xFFu);
    direction.z = float((vertex.x >> 26) & 0x1u);
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

vec3 getNormal(uvec2 vertex)
{
    return Normals[(vertex.y >> 0) & 0x7u];
}

vec2 getTexcoord(uvec2 vertex)
{
    return vec2(float((vertex.y >> 3) & 0xFFu) / 256.0f, 0.5f);
}

void main()
{
    vec3 position = getPosition(inVertex);
    outNormal = getNormal(inVertex);
    outTexcoord = getTexcoord(inVertex);
    gl_Position = viewProjMatrix * modelMatrix * vec4(position, 1.0f);
}