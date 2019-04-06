
#version 440
in vec4 aPos;

smooth out vec4 fragPos;

void main(){
        fragPos=aPos;
        gl_Position = aPos;
}
