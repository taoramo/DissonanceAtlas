#version 330 core
out vec4 FragColor;

in vec3 FragPos;

// A simple color for the terrain
uniform vec3 objectColor;

void main()
{
    FragColor = vec4(FragPos.y * objectColor, 1.0);
}
