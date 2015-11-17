#version 120
uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     time;           // shader playback time (in seconds)

// http://www.pouet.net/prod.php?which=57245
#define t iGlobalTime
#define r iResolution.xy
//         vec2 xy = -1.0 + 2.0*gl_FragCoord.xy/iResolution.xy;
// get the coordinates in gl_FragCoord and write the color to gl_FragColor

void main(){
        vec3 c;
        float l,z=time;
        for(int i=0;i<3;i++) {
                vec2 uv,p=gl_FragCoord.xy/iResolution.xy;
                uv=p;
                p-=.5;
                p.x*=iResolution.x/iResolution.y;
                z+=.07;
                l=length(p);
                uv+=p/l*(sin(z)+1.)*abs(sin(l*9.-z*2.));
                c[i]=.01/length(abs(mod(uv,1.)-.5));
        }
        gl_FragColor=vec4(c/l,time);
        //gl_FragColor=vec4(1.0,0.0,1.0,1.0);
}
