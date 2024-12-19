out vec4 frag_color;

uniform vec4 u_color;
uniform vec2 u_resolution;
uniform double u_width;
uniform double u_time;

layout(binding = 0) uniform sampler2D binded_texture;
layout(binding = 1) uniform sampler2D attachment_texture;

in vec2 v_texture;

void main() {
	vec4 texture_color = texture(binded_texture, v_texture);

	if (texture_color.b > 0.) {
		double d = distance(gl_FragCoord.xy, texture_color.rg);

		if (d < u_width - 1.) {
			frag_color = u_color;
			gl_FragDepth = texture_color.b;
		}
		else {
			if (d < u_width) {
				frag_color = vec4(u_color.rgb, (u_width - d) * u_color.a);
				gl_FragDepth = texture_color.b;
			}
			else { 
				frag_color = vec4(1.0, 0.0, 1.0, 0.0);
				gl_FragDepth = 1.0;
			}
		}
	}
	else {
		frag_color = vec4(1., 0., 1., 0.);
		gl_FragDepth = 1.0;
	}
}