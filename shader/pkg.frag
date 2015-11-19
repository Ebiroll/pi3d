#version 120
varying vec2 texcoord;
uniform sampler2D tex;

void main()
{
    gl_FragColor = texture2D(tex, texcoord);
    //gl_FragColor = vec4(0.0,1.0,0.0,1.0);
    //gl_FragColor = vec4(texcoord.x,texcoord.y,0.0,1.0);
}
