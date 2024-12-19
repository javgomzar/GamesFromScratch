
layout(binding = 0) uniform sampler2D binded_texture;
layout(binding = 1) uniform sampler2D attachment_texture;

uniform vec2 u_resolution;

out vec4 frag_color;

in vec2 v_texture;

void main() {
	vec4 texture_color = texture(binded_texture, v_texture);

	// Inverted
	// texture_color.xyz = vec3(1.0) - texture_color.xyz;

	//	Black and white
	// texture_color.xyz = vec3(texture_color.x + texture_color.y + texture_color.z) / 6.0;

	frag_color = texture_color;
	gl_FragDepth = texture(attachment_texture, v_texture).r;

	// Old computer
	// frag_color = texture_color * vec4(0.5, 1.0, 0.5, Alpha);
}
