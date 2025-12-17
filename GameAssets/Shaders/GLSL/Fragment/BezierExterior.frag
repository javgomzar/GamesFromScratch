#version 450
precision highp float;

layout(location = 0) in vec2 barycentric;
layout(location = 1) in vec4 color;
layout(location = 0) out vec4 frag_color;

void main() {
	float f = 0.5 * barycentric.x + barycentric.y;
    float s = smoothstep(-0.01, 0.01, barycentric.y - f*f);
    frag_color = s * color;
}