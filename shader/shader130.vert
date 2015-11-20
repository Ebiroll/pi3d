#version 130
in vec3 position;
in vec3 normal;
in vec2 texCoords;
in float index;

out vec2 coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
  gl_Position = projection * view * model * vec4(position, 1.0f);
  coords = vec2(texCoords.x, 1.0 - texCoords.y);
}
