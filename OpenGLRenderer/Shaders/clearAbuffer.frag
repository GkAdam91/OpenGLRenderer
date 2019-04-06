/**
 * Fast Single-pass A-Buffer using OpenGL 4.0
 * Copyright Cyril Crassin, June 2010
**/
#version 400
#extension GL_NV_gpu_shader5 : enable
#extension GL_EXT_shader_image_load_store : enable
#extension GL_NV_shader_buffer_load : enable
#extension GL_EXT_bindable_uniform : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_separate_shader_objects : enable
//Macros changed from the C++ side
uniform int  ABUFFER_USE_TEXTURES  = 0 ;
uniform int SCREEN_WIDTH = 1024;
uniform int  SCREEN_HEIGHT =  1024;

//Whole number pixel offsets (not necessary just to test the layout keyword !)
in vec4 gl_FragCoord;




coherent uniform layout(size1x32) uimage2D abufferCounterImg;
coherent uniform layout(size4x32) image2DArray abufferImg;

uniform vec4 *d_abuffer;
uniform uint *d_abufferIdx;
uniform int *d_abufferMeshIdx;

void main(void) {


    ivec2 coords=ivec2(gl_FragCoord.xy);

    //Be sure we are into the framebuffer
    if(coords.x>=0 && coords.y>=0 && coords.x<SCREEN_WIDTH && coords.y<SCREEN_HEIGHT ){
        d_abufferIdx[coords.x+coords.y*SCREEN_WIDTH]=0;
        d_abufferMeshIdx[coords.x+coords.y*SCREEN_WIDTH]=0;
        d_abuffer[coords.x+coords.y*SCREEN_WIDTH]=vec4(0.0f);
    }

    //Discard fragment so nothing is writen to the framebuffer
    discard;

}




