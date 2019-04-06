/**
 * Fast Single-pass A-Buffer using OpenGL 4.0
 * Copyright Cyril Crassin, June 2010
**/

#version 440
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_EXT_bindable_uniform : enable
#extension GL_ARB_separate_shader_objects : enable
//Abuffer Checks
uniform int SCREEN_WIDTH = 1024;
uniform int  SCREEN_HEIGHT =  1024;
uniform float BACKGROUND_COLOR_R =  0.2f;
uniform float BACKGROUND_COLOR_G = 0.2f;
uniform float BACKGROUND_COLOR_B = 0.2f;
#define ABUFFER_SIZE 32
uniform int  ABUFFER_USE_TEXTURES = 0 ;
const float fragmentAlpha=0.5f;
uniform int ABUFFER_RESOLVE_USE_SORTING	= 1;

uniform int ABUFFER_RESOLVE_ALPHA_CORRECTION = 0;
uniform int ABUFFER_RESOLVE_GELLY = 1;
uniform int ABUFFER_DISPNUMFRAGMENTS = 0;
uniform int SHOW_INDEX = 1;


//#include "ABufferSort.hglsl"
//#include "ABufferShading.hglsl"


//Whole number pixel offsets (not necessary just to test the layout keyword !)
in vec4 gl_FragCoord;

//Input interpolated fragment position
smooth in vec4 fragPos;
//Output fragment color
out vec4 outFragColor;

//if ABUFFER_USE_TEXTURES 1
uniform layout(size1x32) uimage2D abufferCounterImg;
uniform layout(size4x32) image2DArray abufferImg;
//else 0
uniform vec4 *d_abuffer;
uniform uint *d_abufferIdx;
uniform int *d_abufferMeshIdx;

vec4 backgroundColor;


//Bubble sort used to sort fragments
void bubbleSort(int array_size);
//Bitonic sort test
void bitonicSort( int n );
//Blend fragments front-to-back
vec4 resolveAlphaBlend(int abNumFrag);
vec4 resolveKBlend(int showIndex, int depth, int abNumFrag);
vec4 resolveKBlend2(int showIndex, int depth, int abNumFrag);
//Compute gelly shader
vec4 resolveGelly(int abNumFrag);
//Local memory array (probably in L1)
vec4 fragmentList[ABUFFER_SIZE];
float meshIndexList[ABUFFER_SIZE];
//Keeps only closest fragment
vec4 resolveClosest(ivec2 coords, int abNumFrag);
//Fill local memory array of fragments
void fillFragmentArray(ivec2 coords, int abNumFrag);

//Resolve A-Buffer and blend sorted fragments
void main(void) {

    backgroundColor=vec4(BACKGROUND_COLOR_R, BACKGROUND_COLOR_G, BACKGROUND_COLOR_B, 0.0f);

    ivec2 coords=ivec2(gl_FragCoord.xy);
    int abNumFrag;
    if(coords.x>=0 && coords.y>=0 && coords.x<SCREEN_WIDTH && coords.y<SCREEN_HEIGHT ){

        //Load the number of fragments in the current pixel.

        abNumFrag=(int)d_abufferIdx[coords.x+coords.y*SCREEN_WIDTH];

        //Crash without this (WTF ??)
        if(abNumFrag<0 )
            abNumFrag=0;
        if(abNumFrag>ABUFFER_SIZE ){
            abNumFrag=ABUFFER_SIZE;
        }
        if(abNumFrag > 0){
            //Copy fragments in local array
            fillFragmentArray(coords, abNumFrag);
            //Sort fragments in local memory array
            bubbleSort(abNumFrag);
            outFragColor = resolveKBlend(SHOW_INDEX, 4, abNumFrag);
        }
        else{
            discard;
        }
    }
}



vec4 resolveClosest(ivec2 coords, int abNumFrag){

    //Search smallest z
    vec4 minFrag=vec4(0.0f, 0.0f, 0.0f, 1000000.0f);
    for(int i=0; i<abNumFrag; i++){
        vec4 val;
        if (ABUFFER_USE_TEXTURES == 1){
            val=imageLoad(abufferImg, ivec3(coords, i));
        }
        else{
            val=d_abuffer[coords.x+coords.y*SCREEN_WIDTH + (i*SCREEN_WIDTH*SCREEN_HEIGHT)];
        }
        if(val.w<minFrag.w){
            minFrag=val;
        }
    }

    //Output final color for the frame buffer
    return minFrag;
}


void fillFragmentArray(ivec2 coords, int abNumFrag){
    //Load fragments into a local memory array for sorting
    for(int i=0; i<abNumFrag; i++){
        if (ABUFFER_USE_TEXTURES == 1){
            fragmentList[i]=imageLoad(abufferImg, ivec3(coords, i));
        }
        else{
            fragmentList[i]=d_abuffer[coords.x+coords.y*SCREEN_WIDTH + (i*SCREEN_WIDTH*SCREEN_HEIGHT)];
            meshIndexList[i]=d_abufferMeshIdx[coords.x+coords.y*SCREEN_WIDTH + (i*SCREEN_WIDTH*SCREEN_HEIGHT)];
        }
    }
}


//Blend fragments front-to-back
vec4 resolveKBlend(int showIndex, int depth, int abNumFrag){

//    for(int i=1; i<abNumFrag; i++){
//        if(meshIndexList[i]<=meshIndexList[i-1]){
//            abNumFrag = i;
//            break;
//        }
//    }

    vec4 finalColor=vec4(0.0,0.0,0.0,1.0);

    if(abNumFrag>3)
        abNumFrag = 3;
    for(int i=0; i<abNumFrag; i++){
        if (i > showIndex -1 )
            continue;

        vec4 frag=fragmentList[i];
        vec4 col;

        if (i == showIndex-1){
            col.rgb = frag.rgb;
            col.w = 0.5f;
        }
        else if (i == showIndex-2){
            col.rgb=frag.rgb;

            col.w = 0.3f;
        }

        else{
            col.rgb=frag.rgb;
            col.w = 0.3f;
        }
        finalColor.rgb = finalColor.rgb + col.rgb*col.a;


    }
    finalColor.a = 1.0f;
    return finalColor;

}

vec4 resolveKBlend2(int showIndex, int depth, int abNumFrag){

    vec4 finalColor=vec4(0.0,0.0,0.0,1.0);

    const float sigma = 30.0f;
    float thickness=fragmentList[0].w/2.0f;

    finalColor=vec4(0.0f);
    if(abNumFrag < showIndex-1){
        finalColor=backgroundColor;
    }
    else{
        for(int i=0; i<abNumFrag; i++){
            //        for(int i=0; i<depth-1; i++){
            if (i > showIndex-1 || i < showIndex-3 )
                continue;

            vec4 frag=fragmentList[i];
            vec4 col;
            if (i == showIndex-1){
                col.rgb = frag.rgb;
                col.r = 1.0f;

                //            col.w = 1.0f;
                col.w = 0.5f;
            }
            else if (i == showIndex-2){
                col.rgb=frag.rgb;
                //            col.w = 0.5/float(abNumFrag-1);
                //            col.w = 0.0f;
                col.w = 0.3f;
            }
            else if (i == showIndex-3){
                col.rgb=frag.rgb;
                //            col.w = 0.5/float(abNumFrag-1);
                col.w = 0.2f;
                //            col.w = 0.0f;
            }
            else{

                col.rgb=frag.rgb;
                //            col.w = 0.5/float(abNumFrag-1);
                col.w = 0.0f;
            }
            finalColor.rgb = finalColor.rgb + col.rgb*col.a;
            //        }

        }
    }
    //finalColor=finalColor+backgroundColor*(1.0f-finalColor.a);
    finalColor.a = 1.0f;


    return finalColor;

}


//Blend fragments front-to-back
vec4 resolveAlphaBlend(int abNumFrag){

    vec4 finalColor=vec4(0.0f);

    const float sigma = 30.0f;
    float thickness=fragmentList[0].w/2.0f;

    finalColor=vec4(0.0f);
    for(int i=0; i<abNumFrag; i++){
        vec4 frag=fragmentList[i];

        vec4 col;
        col.rgb=frag.rgb;
        col.w=fragmentAlpha;	//uses constant alpha

        if(ABUFFER_RESOLVE_ALPHA_CORRECTION == 1){
            if(i%2==abNumFrag%2)
                thickness=(fragmentList[i+1].w-frag.w)*0.5f;
            col.w=1.0f-pow(1.0f-col.w, thickness* sigma );
        }

        col.rgb=col.rgb*col.w;

        finalColor=finalColor+col*(1.0f-finalColor.a);
    }

    finalColor=finalColor+backgroundColor*(1.0f-finalColor.a);

    return finalColor;

}

//Blend fragments front-to-back
vec4 resolveGelly(int abNumFrag){

    float thickness=0.0f;
    vec4 accumColor=vec4(0.0f);

    vec4 prevFrag;
    for(int i=0; i<abNumFrag; i++){
        vec4 frag=fragmentList[i];

        if(i%2==1){
            thickness+=frag.w-prevFrag.w;
        }

        vec4 col;
        col.rgb=frag.rgb;
        col.w=fragmentAlpha;	//uses constant alpha

        col.rgb=col.rgb*col.w;
        accumColor=accumColor+col*(1.0f-accumColor.a);

        prevFrag=frag;
    }
    accumColor=accumColor+backgroundColor*(1.0f-accumColor.a);


    //thickness=fragmentList[abNumFrag-1].w-fragmentList[0].w;
    float sigma = 20.0f;
    float Ia = exp(-sigma*thickness);
    float ka = 0.9f;

    vec4 finalColor=vec4(0.0f);
    finalColor = ka * Ia + (1.0-ka) * fragmentList[0]; //Same as Louis Bavoil 2005
    //finalColor = ka * Ia + (1.0-ka) * accumColor;   //Uses accumulated Color

    const vec4 jade = vec4(0.4f, 0.14f, 0.11f, 1.0f)* 8.0f;
    const vec4 green = vec4(0.3f, 0.7f, 0.1f, 1.0f)* 4.0f;
    const vec4 green2 = vec4(0.14f, 0.45f, 0.05f, 1.0f)* 8.0f;
    finalColor *= green2 ;

    return finalColor;
}

//Bubble sort used to sort fragments
void bubbleSort(int array_size) {
    for (int i = (array_size - 2); i >= 0; --i) {
        for (int j = 0; j <= i; ++j) {
            if (fragmentList[j].w > fragmentList[j+1].w) {
                vec4 temp = fragmentList[j+1];
                fragmentList[j+1] = fragmentList[j];
                fragmentList[j] = temp;

                float temp2= meshIndexList[j+1];
                meshIndexList[j+1] = meshIndexList[j];
                meshIndexList[j] = temp2;

            }
        }
    }
}

//Swap function
void swapFragArray(int n0, int n1){
    vec4 temp = fragmentList[n1];
    fragmentList[n1] = fragmentList[n0];
    fragmentList[n0] = temp;
}

//->Same artefact than in L.Bavoil
//Bitonic sort: http://www.tools-of-computing.com/tc/CS/Sorts/bitonic_sort.htm
void bitonicSort( int n ) {
    int i,j,k;
    for (k=2;k<=n;k=2*k) {
        for (j=k>>1;j>0;j=j>>1) {
            for (i=0;i<n;i++) {
                int ixj=i^j;
                if ((ixj)>i) {
                    if ((i&k)==0 && fragmentList[i].w>fragmentList[ixj].w){
                        swapFragArray(i, ixj);
                    }
                    if ((i&k)!=0 && fragmentList[i].w<fragmentList[ixj].w) {
                        swapFragArray(i, ixj);
                    }
                }
            }
        }
    }
}
