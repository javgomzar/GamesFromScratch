
uniform sampler2D binded_texture;

out vec4 frag_color;
uniform vec4 u_color;
uniform vec2 u_resolution;

in vec2 v_texture;

void main() {
	float Height = gl_FragCoord.y;
	frag_color = vec4(mix(vec3(0.), u_color.rgb, 0.4 * Height / u_resolution.y + 0.5), 1.0);
}
