#version 430 core
precision highp float;

layout(binding = 0) uniform sampler2D binded_texture;
layout(binding = 1) uniform sampler2D attachment_texture;

out vec4 frag_color;

in vec2 v_texture;

void main() {
	vec4 texture_color = texture(binded_texture, v_texture);

	frag_color = texture_color;
	gl_FragDepth = texture(attachment_texture, v_texture).r;
}
