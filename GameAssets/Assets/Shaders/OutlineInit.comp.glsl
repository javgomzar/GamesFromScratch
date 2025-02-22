
layout(binding = 0) uniform sampler2D binded_texture;
layout(binding = 1) uniform sampler2D attachment_texture;

out vec4 frag_color;

in vec2 v_texture;

void main() {
	vec4 texture_color = texture(binded_texture, v_texture);

	float depth = texture(attachment_texture, v_texture).r + 0.005;

	if (texture_color.r > 0.99) {
		frag_color = vec4(gl_FragCoord.xy, depth, 1.0);
	}
	if (texture_color.r < 0.01) {
		frag_color = vec4(-1., 0., 0., 1.);
	}
	else {
		mat3 sobel_x = mat3(
			vec3(-1., 0., 1.), 
			vec3(-2., 0., 2.),
			vec3(-1., 0., 1.)
		);
		mat3 sobel_y = mat3(
			vec3(-1.,-2.,-1.),
			vec3( 0., 0., 0.),
			vec3( 1., 2., 1.)
		);

		float offset_x = 0;
		float offset_y = 0;

		for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			vec2 p = (gl_FragCoord.xy + vec2(x-1,y-1)) / u_resolution.xy;
			float value = texture(binded_texture, p).r;
			offset_x += sobel_x[y][x] * value;
			offset_y += sobel_y[y][x] * value;
		}}

		if (abs(offset_x) <= 0.005 && abs(offset_y) < 0.005) {
			frag_color = vec4(gl_FragCoord.xy, depth, 1.0);
		}
		else {
			vec2 offset = (1. - texture_color.r) * normalize(vec2(offset_x, offset_y));
			frag_color = vec4(gl_FragCoord.xy + offset, depth, 1.);
		}
	}
}
