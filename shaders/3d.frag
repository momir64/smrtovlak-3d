#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool applyGreenTint;
uniform sampler2D texture1;
uniform bool useTexture;
uniform vec3 baseColor;

uniform bool screenGreenTint;
uniform vec2 resolution;

void main() {
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float dist = length(lightPos - FragPos);
    float attenuation = smoothstep(500.0, 0.0, dist);
    diffuse *= attenuation;
    
    vec3 objectColor = useTexture ? texture(texture1, TexCoord).rgb : baseColor;
    
    if (applyGreenTint)
        objectColor = objectColor * vec3(0.4, 1.3, 0.5);
    
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);

    if (screenGreenTint) 
        FragColor.rgb = FragColor.rgb * vec3(0.5, 1.0, 0.6);    
}
