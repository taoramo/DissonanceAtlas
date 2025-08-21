#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 ViewFragPos;
in vec3 ModelFragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform float heightMultiplier;
uniform vec3 lightColor;

void main()
{
    // Color gradient based on MODEL space height
    float height = ModelFragPos.y / (heightMultiplier * 1.0);

    // Create a metallic gradient from deep blue to red
    vec3 color1 = vec3(0.05, 0.1, 0.3);  // Deep blue (valleys)
    vec3 color2 = vec3(0.3, 0.2, 0.5);   // Deep purple (low-mid)
    vec3 color3 = vec3(0.5, 0.1, 0.7);   // Purple (mid-level)
    vec3 color4 = vec3(0.8, 0.2, 0.4);   // Magenta-red (highlands)
    vec3 color5 = vec3(1.0, 0.3, 0.3);   // Bright red (peaks)

    vec3 objectColor = mix(color1, color2, smoothstep(0.0, 0.25, height));
    objectColor = mix(objectColor, color3, smoothstep(0.25, 0.5, height));
    objectColor = mix(objectColor, color4, smoothstep(0.5, 0.75, height));
    objectColor = mix(objectColor, color5, smoothstep(0.75, 1.0, height));

    // --- Lighting Calculations in VIEW space ---

    vec3 norm = normalize(Normal);

    // fragment to light
    vec3 lightDir = normalize(lightPos - ViewFragPos);

    float ambientStrength = 0.0;
    vec3 ambient = ambientStrength * lightColor;

    float diff = max(dot(norm, lightDir), 0.0);
    diff = pow(diff, 0.6);
    vec3 diffuse = diff * lightColor * 1.5;

    float specularStrength = 1.2;
    vec3 viewDir = normalize(-ViewFragPos); // In view space, camera is at (0,0,0)
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    vec3 specularColor = mix(lightColor, objectColor, 0.3);
    vec3 specular = specularStrength * spec * specularColor;

    float rim = 1.0 - max(dot(viewDir, norm), 0.0);
    rim = pow(rim, 2.0);
    vec3 rimLighting = rim * lightColor * 0.6;

    vec3 lighting = ambient + diffuse + specular + rimLighting;

    vec3 result = lighting * objectColor;

    float metallicBrightness = 1.0 + 0.5 * height;
    result *= metallicBrightness;

    result = min(result, vec3(1.0));

    FragColor = vec4(result, 1.0);
}
