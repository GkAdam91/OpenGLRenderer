#version 440 core

#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_EXT_bindable_uniform : enable
#extension GL_ARB_separate_shader_objects : enable

//#include "ABufferSort.hglsl"
//#include "ABufferShading.hglsl"


out vec4 outFragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
flat in int meshIndex;

smooth in vec4 fragPos;
smooth in vec3 fragTexCoord;
smooth in vec3 fragNormal;


// material parameters
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_roughness1;
uniform sampler2D aoMap;
uniform sampler2D defaultTexture;

layout(pixel_center_integer) in vec4 gl_FragCoord;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

//booleans from checkboxes
uniform int diffBool;
uniform int normBool;
uniform int specBool;
uniform int isMetallic;
uniform int roughBool=0;
uniform int aoBool=0;

uniform float specVal;
uniform float roughVal;
uniform float aoVal;

//ABuffer Checks
uniform int USE_ABUFFER	= 1;
uniform int SCREEN_WIDTH = 1024;
uniform int  SCREEN_HEIGHT =  1024;
uniform float BACKGROUND_COLOR_R =  0.2f;
uniform float BACKGROUND_COLOR_G = 0.2f;
uniform float BACKGROUND_COLOR_B = 0.2f;
uniform int SHOW_INDEX = 1;

uniform int ABUFFER_RESOLVE_GELLY = 1;
uniform int  ABUFFER_USE_TEXTURES = 0;
#define ABUFFER_SIZE 32

//A-Buffer fragments storage
coherent uniform layout(size4x32) image2DArray abufferImg;
//A-Buffer fragment counter
coherent uniform layout(size1x32) uimage2D abufferCounterImg;
//no Textures buffers

uniform vec4 *d_abuffer;
uniform coherent uint *d_abufferIdx;
uniform int *d_abufferMeshIdx;



const vec4 backgroundColor=vec4(BACKGROUND_COLOR_R, BACKGROUND_COLOR_G, BACKGROUND_COLOR_B, 0.0f);
const float fragmentAlpha=0.5f;

//Local memory array (probably in L1)
vec4 fragmentList[ABUFFER_SIZE];




uniform vec3 camPos;

const float PI = 3.14159265359;


vec3 getNormalFromMap()
{
    vec3 tangentNormal;
    if (normBool == 1){
        tangentNormal = texture(texture_normal1, TexCoords).xyz * 2.0 - 1.0;
        vec3 Q1  = dFdx(WorldPos);
        vec3 Q2  = dFdy(WorldPos);
        vec2 st1 = dFdx(TexCoords);
        vec2 st2 = dFdy(TexCoords);

        vec3 N   = normalize(Normal);
        vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
        vec3 B  = -normalize(cross(N, T));
        mat3 TBN = mat3(T, B, N);

        return normalize(TBN * tangentNormal);
    }
    else
    {
        return normalize(Normal);
    }


}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

//Shade using green-white strips
vec3 shadeStrips(vec3 texcoord){
    vec3 col;
    float i=floor(texcoord.x*6.0f);

    col.rgb=fract(i*0.5f) == 0.0f ? vec3(0.4f, 0.85f, 0.0f) : vec3(1.0f);
    col.rgb*=texcoord.z;

    return col;
}

vec3 computeAlbedo(){
    vec3 albedo = pow(texture(texture_diffuse1, TexCoords).rgb, vec3(2.2));
    return albedo;
}

vec3 computePBR(){
    vec3 albedo;
    if (diffBool == 1){
        albedo  = pow(texture(texture_diffuse1, TexCoords).rgb, vec3(2.2));
    }
    else{
        albedo = vec3(0.2,0.2,0.2);
    }

    float metallic;
    if (specBool == 1){
        metallic  = texture(texture_specular1, TexCoords).r;

    }
    else{
        metallic = specVal;
    }
    //float roughness = texture(roughnessMap, TexCoords).r;
    //float ao        = texture(aoMap, TexCoords).r;
    float roughness;
    if (roughBool == 1){
        roughness  = texture(texture_roughness1, TexCoords).r;
    }
    else{
        roughness = roughVal;
    }

    float ao;
    if (aoBool == 1){
        ao  = texture(aoMap, TexCoords).r;
//        ao = 0.0;

    }
    else{
        ao = 1.0;
    }

    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; i++)
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance) * 1000.0;
        vec3 radiance = lightColors[i] * attenuation ;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator    = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 specular = nominator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic ;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (note that the next IBL tutorial will replace
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * albedo ;//* ao;
    //    vec3 color = ambient;
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));
    //    color = albedo;
    return color;
}

void main()
{
    if(USE_ABUFFER == 1){
        ivec2 coords=ivec2(gl_FragCoord.xy);
        //Check we are in the framebuffer
        if(coords.x>=0 && coords.y>=0 && coords.x<SCREEN_WIDTH && coords.y<SCREEN_HEIGHT){
            int abidx;

            //vec3 albedo;
            if (ABUFFER_USE_TEXTURES==1){
                //albedo  = pow(texture(texture_diffuse1, TexCoords).rgb, vec3(2.2));
                abidx=(int)imageAtomicIncWrap(abufferCounterImg, coords, ABUFFER_SIZE );
            }
            else{
                //albedo = vec3(0.5,0.5,0.5);
                abidx=(int)atomicIncWrap(d_abufferIdx+coords.x+coords.y*SCREEN_WIDTH, ABUFFER_SIZE);
            }


            vec4 abuffval;
            vec3 col;

//            if(SHOW_INDEX == meshIndex-1){
//                col = computePBR();
//            }
//            else{
//                //                col = vec3(1.0,0.0,0.0);
//                computeAlbedo();
//            }
                col = computePBR();
            abuffval.rgb=col;
            abuffval.w=fragPos.z;	//Will be used for sorting
            //Put fragment into A-Buffer
            if (ABUFFER_USE_TEXTURES==0){
                int temp = coords.x+coords.y*SCREEN_WIDTH + (abidx*SCREEN_WIDTH*SCREEN_HEIGHT);
                d_abuffer[temp]=abuffval;
                d_abufferMeshIdx[temp] = meshIndex;
            }
            else{
                imageStore(abufferImg, ivec3(coords, abidx), abuffval);
            }
        }
        discard;
    }
    else{  //Do not Use A-buffer
        vec3 col=shadeStrips(fragTexCoord);
        outFragColor=vec4(1.0f,1.0f,1.0f, 1.0f);
    }
}

