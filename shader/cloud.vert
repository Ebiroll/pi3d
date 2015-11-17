attribute vec3 position;
attribute vec3 normal;
attribute vec2 vtex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

varying vec2 vUv;


void main() {
        vUv = vtex;
        gl_Position = projection * view * model * vec4( position, 1.0 );
}
