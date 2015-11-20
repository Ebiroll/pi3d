#version 130
in vec2 coords;
out vec4 color;

uniform sampler2D tex;

void main()
{    
    //gl_FragColor =  texture2D(tex, coords);
    color=texture(tex, coords);
    //color =  vec4 (coords.x,coords.y,0.0,1.0);
}
