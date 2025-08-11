#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 FragPos;

uniform mat4 mvp;

uniform sampler2D heightMap;
uniform float heightMultiplier;

void main()
{
    TexCoords = aTexCoords;

    // Sample the heightmap to get the displacement value
    float height = texture(heightMap, aTexCoords).r;

    // Displace the vertex's Y position
    vec3 displacedPos = aPos + vec3(0.0, height * heightMultiplier, 0.0);

    gl_Position = mvp * vec4(displacedPos, 1.0);
}
