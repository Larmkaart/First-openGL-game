#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec4 VertexCoords;
in vec3 VertexNormal;


uniform sampler2D text;
uniform vec3 viewPos;
//uniform vec4 sunPosition;

float getFog(float d, float near, float far)
{
    float fMax = 1.0f * far;
    float fMin = 0.5f * far;
    if (d >= fMax ) return 1.0f;
    if (d <= fMax ) return 0.0f;

}

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

    // fog
    float near = 250.0f;
    float far = 1500.0f;
    vec3 fogColor = vec3(1.0f);

    vec3 relCamPos = VertexCoords.xyz - viewPos;
    float sqrtDistToCam = dot(relCamPos, relCamPos);

    float fog = max((sqrtDistToCam - near * near) / (far * far), 0.0f);
    result += fogColor * fog * fog;

    FragColor = vec4(result, 1.0f);
}