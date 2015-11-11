uniform sampler2D texture;
varying vec4 vColor;
varying float vAngle; 
void main()
{
	gl_FragColor = vColor;
			
	float c = cos(vAngle);
	float s = sin(vAngle);
	vec2 rotatedUV = vec2(c * (gl_PointCoord.x - 0.5) + s * (gl_PointCoord.y - 0.5) + 0.5,
        c * (gl_PointCoord.y - 0.5) - s * (gl_PointCoord.x - 0.5) + 0.5);  // rotate UV coordinates to rotate texture
		    
       vec4 rotatedTexture = texture2D( texture,  rotatedUV );
	gl_FragColor = gl_FragColor * rotatedTexture;
}
