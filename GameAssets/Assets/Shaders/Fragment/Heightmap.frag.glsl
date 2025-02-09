#version 430 core

in float height;

out vec4 frag_color;

void main() {
	float h = (height + 16)/64.0f;
	frag_color = vec4(vec3(h), 1.0);
}