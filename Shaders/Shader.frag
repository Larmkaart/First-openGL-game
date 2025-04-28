#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec4 VertexCoords;
in vec3 VertexNormal;
in float WaterLevel;


uniform sampler2D text;
uniform vec3 viewPos;
uniform float MinHeight;
uniform float MaxHeight;

vec3 result;

vec4 sunPosition = vec4(0.0f, 100.0f, 100.0f, 1.0f);
vec3 lightColor = vec3(1.0f);
float ambientStrenght = 0.9f;
float near = 500.0f;
float far = 1500.0f;
vec3 fogColor = vec3(0.8f);

//uniform vec4 sunPosition;

float getFog(float d, float near, float far)
{
    float fMax = 1.0f * far;
    float fMin = 0.5f * far;
    if (d >= fMax ) return 1.0f;
    if (d <= fMax ) return 0.0f;

}

struct TerrainType
{
    vec3 TerrainColor;
    float TerrainLevel;
};

TerrainType CT(vec3 col, float lvl)
{
    TerrainType tr;
    tr.TerrainColor = col;
    tr.TerrainLevel = lvl;
    return tr;
}

struct Terrain
{
    TerrainType types[6];
};

vec3 getColorMap(float height, Terrain terrain)
{
    if (height < terrain.types[0].TerrainLevel)
        return terrain.types[0].TerrainColor;

    for (int i = 1; i < 6; i++)
    {
        TerrainType t = terrain.types[i];
        if (height < t.TerrainLevel)
        {
            vec3 oldCol = terrain.types[i-1].TerrainColor;
            float oldLvl = terrain.types[i-1].TerrainLevel;
            float mx = smoothstep(oldLvl, t.TerrainLevel, height);
            return mix(oldCol, t.TerrainColor, mx);
        }
    }

    return terrain.types[5].TerrainColor;
}

void main()
{   
    Terrain terrain;
    terrain.types[0] = CT(vec3(0.0, 0.4, 0.7), 0.4);    // Deep Water (rich ocean blue)
    terrain.types[1] = CT(vec3(0.85, 0.8, 0.5), 0.5);   // Sand (warmer beach tone)
    terrain.types[2] = CT(vec3(0.2, 0.7, 0.2), 0.6);    // Grassland (natural green)
    terrain.types[3] = CT(vec3(0.1, 0.5, 0.1), 0.75);   // Forest (darker forest green)
    terrain.types[4] = CT(vec3(0.4, 0.4, 0.4), 0.9);    // Rock (true rock gray)
    terrain.types[5] = CT(vec3(1.0, 1.0, 1.0), 1.1);    // Snow (pure white)
    
    // Ambient
    vec3 ambient = lightColor * ambientStrenght;

    // Diffuse
    vec3 normal = normalize(VertexNormal);
    vec3 lightDir = normalize(sunPosition - VertexCoords).xyz;
    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = lightColor * diff * 0.2f;

    if (VertexCoords.y <= WaterLevel)
        result = vec3(30.0f/255.0f, 150.0f/255.0f, 150.0f/255.0f);
    else
        result = (diffuse + ambient) * mix(texture(text, TexCoords).xyz, getColorMap(1 - (VertexCoords.y - MinHeight - 200) / (MaxHeight - 100), terrain), 1.0f);

    // fog
    vec3 relCamPos = VertexCoords.xyz - viewPos;
    float sqrtDistToCam = dot(relCamPos, relCamPos);
    float fog = max((sqrtDistToCam - near * near) / (far * far), 0.0f);
    result += fogColor * fog * fog;

    FragColor = vec4(result, 1.0f);
}