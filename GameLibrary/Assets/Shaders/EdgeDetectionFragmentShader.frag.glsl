out vec4 frag_color;

uniform vec4 u_color;
uniform vec2 u_resolution;
uniform double u_width;
uniform double u_time;

uniform sampler2D binded_texture;

in vec2 v_texture;

void main() {
	vec4 texture_color = texture(binded_texture, v_texture);

	mat3 sobel_x = mat3(vec3(-1., -2., -1.),vec3(0.),vec3(1., 2., 1.));
	mat3 sobel_y = mat3(vec3(-1., 0., 1.), vec3(-2.,0.,2.),vec3(-1., 0., 1.));
	mat3 sobel_diag_1 = mat3(vec3(-2., -1., 0.), vec3(-1., 0., 1.), vec3(0., 1., 2.));
	mat3 sobel_diag_2 = mat3(vec3(0., -1., -2.), vec3(1., 0., -1.), vec3(2., 1., 0.));

	float value_x = 0;
	float value_y = 0;
	float diag_1 = 0;
	float diag_2 = 0;

	for (int y = 0; y <= 2; ++y) {
	for (int x = 0; x <= 2; ++x) {
		vec2 p = (gl_FragCoord.xy + vec2(x-1,y-1)) / u_resolution.xy;
		float value = texture(binded_texture, p).r;
		value_x += sobel_x[y][x] * value;
		value_y += sobel_y[y][x] * value;
		diag_1 += sobel_diag_1[y][x] * value;
		diag_2 += sobel_diag_2[y][x] * value;
	}}
	
	frag_color = vec4(0.25 * texture_color.r * vec2(abs(value_x), abs(value_y)), 0, 1.0);
}