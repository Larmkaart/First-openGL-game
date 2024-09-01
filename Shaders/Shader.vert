#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec4 VertexCoords;
out vec3 VertexNormal;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;    
    VertexNormal = normalize(mat3(transpose(inverse(model))) * aNormal);
    VertexCoords = vec4(aPos, 1.0);
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}