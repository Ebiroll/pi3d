uniform sampler2D tex;
uniform vec3 fogColor;
uniform float fogNear;
uniform float fogFar;
uniform float opacity;

varying vec2 vUv;

void main() {

	float depth = gl_FragCoord.z / gl_FragCoord.w;
	float fogFactor = smoothstep( fogNear, fogFar, depth );

        gl_FragColor = texture2D( tex, vUv );
	gl_FragColor.w *= pow( gl_FragCoord.z, 20.0 );
	gl_FragColor.w *= opacity;
	gl_FragColor = mix( gl_FragColor, vec4( fogColor, gl_FragColor.w ), fogFactor );

}
