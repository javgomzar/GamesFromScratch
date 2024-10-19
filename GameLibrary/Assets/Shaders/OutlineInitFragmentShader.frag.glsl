
uniform sampler2D binded_texture;
uniform vec2 u_resolution;

out vec4 frag_color;

in vec2 v_texture;

void main() {
	vec4 TextureColor = texture(binded_texture, v_texture);

	if (TextureColor.r == 1.) {
		frag_color = vec4(gl_FragCoord.xy, 0.0, 1.0);
	}
	else {
		frag_color = vec4(-1., 0., 1., 1.);
	}

}
