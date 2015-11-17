#version 120
attribute vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{     
  gl_Position =  projection * view  * vec4(position.x,position.y,position.z,1.0);
}


