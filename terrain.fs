#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;

// A simple color for the terrain
uniform vec3 objectColor;

void main()
{
    FragColor = vec4(0, FragPos.y, 0, 1);
}
