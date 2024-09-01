#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec4 VertexCoords;
in vec3 VertexNormal;


uniform sampler2D text;
//uniform vec4 sunPosition;

void main()
{    
    vec4 sunPosition = vec4(0.0f, 100.0f, 100.0f, 1.0f);
    vec3 lightColor = vec3(1.0f);
    // Ambient
    float ambientStrenght = 0.9f;
    vec3 ambient = lightColor * ambientStrenght;

    // Diffuse
    vec3 normal = normalize(VertexNormal);
    vec3 lightDir = normalize(sunPosition - VertexCoords).xyz;
    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = lightColor * diff * 0.2f;

    vec3 result = (diffuse + ambient) * texture(text, TexCoords).xyz;
    FragColor = vec4(result, 1.0f);
}