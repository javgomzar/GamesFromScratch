#version 450
precision highp float;

layout (location = 1) in vec4 color;
layout (location = 0) out vec4 frag_color;

void main() {
	frag_color = color;
}