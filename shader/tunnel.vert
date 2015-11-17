attribute vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{     
  // skip, model
  gl_Position = projection * view *  vec4(position.x,position.y,0.0,1.0);
}


