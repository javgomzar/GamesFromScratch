#version 430
precision highp float;

uniform vec3 light_direction;
uniform vec3 light_color;
uniform float light_ambient;
uniform float light_diffuse;

out vec4 frag_color;

in vec3 v_normal;
in float height;

void main() {
    vec4 blue = vec4(0.04, 0.22, 0.25, 1.0);
    vec4 white = vec4(0.4706, 0.6314, 0.7922, 1.0);
    frag_color = mix(blue, white, (height + 0.5));
}