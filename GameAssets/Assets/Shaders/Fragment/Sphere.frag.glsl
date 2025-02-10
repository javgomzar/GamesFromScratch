#version 430
precision highp float;

uniform vec3 light_direction;
uniform vec3 light_color;
uniform float light_ambient;
uniform float light_diffuse;

uniform vec4 u_color;

in vec3 v_position;

out vec4 frag_color;

void main() {
	float diffuse = light_diffuse * max(dot(normalize(v_position), -light_direction), 0.0);
	frag_color = u_color * vec4((light_ambient + diffuse) * light_color, 1.0);
}