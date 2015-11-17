uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     time;           // shader playback time (in seconds)
//
//  Tunnel effect by mixing 2 textures
//         vec2 xy = -1.0 + 2.0*gl_FragCoord.xy/iResolution.xy;
// get the coordinates in gl_FragCoord (fragCoord) and write the color to gl_FragColor (fragColor), time (iGlobalTime)
// tex (iChannel0)

uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{
        vec2 p = gl_FragCoord.xy / iResolution.xy;
        vec2 q = p - vec2(0.5, 0.5);

        q.x += sin(time* 0.6) * 0.2;
        q.y += cos(time* 0.4) * 0.3;

        float len = length(q);

        float a = atan(q.y, q.x) + time * 0.3;
        float b = atan(q.y, q.x) + time * 0.3;
        float r1 = 0.3 / len + time * 0.5;
        float r2 = 0.2 / len + time * 0.5;

        float m = (1.0 + sin(time * 0.5)) / 2.0;
        vec4 tex1 = texture2D(tex, vec2(a + 0.1 / len, r1 ));
        vec4 tex2 = texture2D(tex2, vec2(b + 0.1 / len, r2 ));
        vec3 col = vec3(mix(tex1, tex2, m));
        gl_FragColor = vec4(col * len * 1.5, 1.0);
}
