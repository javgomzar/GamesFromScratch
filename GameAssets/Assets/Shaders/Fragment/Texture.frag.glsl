#version 430 core

uniform sampler2D binded_texture;

uniform vec4 u_color;

out vec4 frag_color;

in vec2 v_texture;

void main() {
	frag_color = u_color * texture(binded_texture, v_texture);
}