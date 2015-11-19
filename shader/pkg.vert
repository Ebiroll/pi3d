#version 120
attribute vec3 position;
attribute vec3 normal;
attribute vec2 vtex;
attribute float index;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

varying vec2 texcoord;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    //texcoord = vec2(vtex.x , vtex.y)  + vec2(0.5);
    texcoord  =  vtex;
    //texcoord = vec2(vtex.x, 1.0 - vtex.y);
}
