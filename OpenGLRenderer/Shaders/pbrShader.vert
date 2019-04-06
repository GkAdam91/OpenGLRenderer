#version 440 core
in vec3 aPos;
in vec3 aNormal;
in vec2 aTexCoords;
in int meshIndx;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
flat out int meshIndex;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 modelIT;
smooth out vec4 fragPos;
smooth out vec3 fragTexCoord;

smooth out vec3 fragNormal;
void main()
{
    TexCoords = aTexCoords;
    WorldPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(model) * aNormal;
    fragNormal = normalize((vec4(Normal, 1.0f)*modelIT).xyz);
    fragTexCoord.xy=aPos.xy;
    fragTexCoord.z=abs(fragNormal.z);
    meshIndex = meshIndx;

    fragPos = projection * view * vec4(WorldPos, 1.0);
    gl_Position =  fragPos;

}
