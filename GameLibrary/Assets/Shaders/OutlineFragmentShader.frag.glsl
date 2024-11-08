out vec4 frag_color;

uniform vec4 u_color;
uniform vec2 u_resolution;
uniform double u_width;
uniform double u_time;

uniform sampler2D binded_texture;

in vec2 v_texture;

void main() {
	vec4 texture_color = texture(binded_texture, v_texture);

	/*
	double d = Distance(gl_FragCoord.xy, texture_color.rg) - 20. * u_time;
	vec3 color = hsv2rgb(vec3(fract(d / 100.), 1.0, 1.0));
	frag_color = vec4(color, 1.);
	*/

	if (texture_color.b == 0.) {
		double d = distance(gl_FragCoord.xy, texture_color.rg);

		//if (d <= u_width) {
		//	frag_color = u_color;
		//}
		//else {
		//	frag_color = vec4(1.0, 0.0, 1.0, 0.0);
		//}

		if (d < u_width - 1.) {
			frag_color = u_color;
		}
		else {
			if (d < u_width) frag_color = vec4(u_color.rgb, u_width - d);
			else frag_color = vec4(1.0, 0.0, 1.0, 0.0);
		}
	}
	else {
		frag_color = vec4(1., 0., 1., 0.);
	}
}