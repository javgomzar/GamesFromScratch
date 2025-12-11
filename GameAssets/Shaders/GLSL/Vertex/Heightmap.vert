#version 450
precision highp float;

layout(location = 0) in vec2 a_position;
layout(location = 0) out vec2 v_position;

void main() {
	gl_Position = vec4(a_position.x, 0.0, a_position.y, 1.0);
	v_position = a_position;
}