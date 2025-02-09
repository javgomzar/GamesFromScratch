#version 430
precision highp float;

uniform sampler2D binded_texture;

uniform mat3 u_kernel;
uniform vec2 u_resolution;

out vec4 frag_color;

in vec2 v_texture;

void main() {
	vec4 result = vec4(0);
	for (int y = -1; y <= 1; y++) {
		for (int x = -1; y <= 1; x++) {
			vec2 t = v_texture + vec2(x, y) / u_resolution;
			if (t.x >= 0. && t.y >= 0. && t.x <= 1. && t.y <= 1.) {
				vec4 color = texture(binded_texture, t);
				result += u_kernel[y + 1][x + 1] * color;
			}
		}
	}

	frag_color = result;
}
