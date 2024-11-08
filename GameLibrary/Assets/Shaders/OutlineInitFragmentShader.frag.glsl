
uniform sampler2D binded_texture;
uniform vec2 u_resolution;

out vec4 frag_color;

in vec2 v_texture;

void main() {
	vec4 texture_color = texture(binded_texture, v_texture);

	if (texture_color.r == 1.) {
		frag_color = vec4(gl_FragCoord.xy, 0.0, 1.0);
	}
	else if (texture_color.r == 0.) {
		frag_color = vec4(-1., 0., 1., 1.);
	}
	else {
		mat3 sobel_x = mat3(vec3(-1., -2., -1.),vec3(0.),vec3(1., 2., 1.));
		mat3 sobel_y = mat3(vec3(-1., 0., 1.), vec3(-2.,0.,2.),vec3(-1., 0., 1.));

		float offset_x = 0;
		float offset_y = 0;

		for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			vec2 p = (gl_FragCoord.xy + vec2(x-1,y-1)) / u_resolution.xy;
			float value = texture(binded_texture, p).r;
			offset_x += sobel_x[y][x] * value;
			offset_y += sobel_y[y][x] * value;
		}}

		vec2 offset = normalize(vec2(offset_x, offset_y));

		frag_color = vec4(gl_FragCoord.xy -	texture_color.r * offset, 0., 1.);
	}

}
