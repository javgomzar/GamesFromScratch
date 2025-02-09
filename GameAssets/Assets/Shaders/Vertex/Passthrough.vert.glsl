#version 430
precision highp float;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texture;

out vec3 v_position;
out vec2 v_texture;

void main() {
	gl_Position = vec4(a_position, 1.0);
	v_position = a_position;
	v_texture = a_texture;
}