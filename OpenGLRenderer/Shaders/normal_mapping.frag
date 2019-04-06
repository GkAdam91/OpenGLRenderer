#version 440 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
} ;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

uniform sampler2D texture_diffuse1_rendered;
uniform sampler2D texture_default;

uniform int diffBool;
uniform int normBool;
uniform int specBool;


uniform vec3 viewPos;
uniform Light lights[4];
void main()
{
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < 4; i++){
        vec3 lightDir = normalize(lights[i].position - fs_in.FragPos);

        // check if lighting is inside the spotlight cone

        // obtain normal from normal map in range [0,1]
        vec3 normal;
        if (normBool == 1){
            normal = texture(texture_normal1, fs_in.TexCoords).rgb;
        }
        else{
            normal = vec3(1.0,1.0,1.0);
        }
        // transform normal vector to range [-1,1]
        normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
        vec3 Matcolor;
        // get diffuse color
        if (diffBool == 1){
            Matcolor = texture(texture_diffuse1, fs_in.TexCoords).rgb;
        }
        else{
            Matcolor = vec3(0.5,0.5,0.5);
        }
        // ambient
        vec3 ambient1 = lights[i].ambient * Matcolor;

        // diffuse
        vec3 norm =  normalize(normal);
        float diff = max(dot(lightDir, norm), 0.0);
        vec3 diffuse = lights[i].diffuse * diff * Matcolor;

        // specular
        vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, reflectDir), 0.0), 32.0);

        vec3 Matspecular = texture(texture_specular1, fs_in.TexCoords).rgb;
        vec3 specular = lights[i].specular * spec * Matspecular;

        // attenuation
        float distance    = length(lights[i].position - fs_in.FragPos);
        float attenuation = 1.0 / (lights[i].constant + lights[i].linear * distance + lights[i].quadratic * (distance * distance));

        // ambient  *= attenuation; // remove attenuation from ambient, as otherwise at large distances the light would be darker inside than outside the spotlight due the ambient term in the else branche
        diffuse   *= attenuation;
        specular *= attenuation;
        Lo += ambient1 + diffuse + specular;
    }
    FragColor = vec4(Lo, 1.0);
}
