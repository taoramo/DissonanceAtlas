#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 ViewFragPos;
out vec3 ModelFragPos;
out vec3 Normal;

uniform mat4 mvp;
uniform mat4 modelView;
uniform mat3 normalMatrix;

uniform sampler2D heightMap;
uniform float heightMultiplier;
uniform float textureSize;
uniform float worldPlaneSize;

// Calculates the surface normal using the gradient of the heightmap.
// This method is independent of the mesh resolution.
vec3 calculateNormal(vec2 texCoords) {
    float texelSize = 1.0 / textureSize; // The size of one pixel in texture coordinates
    float worldTexelSize = worldPlaneSize / textureSize; // The size of one pixel in world coordinates

    // Sample the height at the current point and its neighbors
    float h = texture(heightMap, texCoords).r;
    float hR = texture(heightMap, vec2(texCoords.x + texelSize, texCoords.y)).r;
    float hL = texture(heightMap, vec2(texCoords.x - texelSize, texCoords.y)).r;
    float hU = texture(heightMap, vec2(texCoords.x, texCoords.y + texelSize)).r;
    float hD = texture(heightMap, vec2(texCoords.x, texCoords.y - texelSize)).r;

    // Use central differences for better normal calculation
    float dx = (hR - hL) * heightMultiplier / (2.0 * worldTexelSize);
    float dz = (hU - hD) * heightMultiplier / (2.0 * worldTexelSize);

    // Create normal vector (negate dz for correct orientation)
    vec3 normal = normalize(vec3(-dx, 1.0, -dz));

    return normal;
}

void main()
{
    TexCoords = aTexCoords;

    float height = texture(heightMap, aTexCoords).r;
    vec3 displacedPos = aPos + vec3(0.0, height * heightMultiplier, 0.0);
    
    ViewFragPos = vec3(modelView * vec4(displacedPos, 1.0));
    ModelFragPos = displacedPos;

    Normal = normalMatrix * calculateNormal(aTexCoords);

    gl_Position = mvp * vec4(displacedPos, 1.0);
}
