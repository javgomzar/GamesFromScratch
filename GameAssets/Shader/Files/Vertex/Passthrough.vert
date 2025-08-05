#version 450
precision highp float;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texture;

layout(location = 0) out vec3 v_position;
layout(location = 1) out vec2 v_texture;

void main() {
	gl_Position = vec4(a_position, 1.0);
	v_position = a_position;
	v_texture = a_texture;
}