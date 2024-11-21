layout(binding = 0) uniform sampler2DMS sampler;

uniform int u_samples;
uniform vec2 u_resolution;

in vec2 v_texture;

out vec4 frag_color;

void main() {
	//vec4 texture_color = texture(binded_texture, gl_FragCoord.xy / u_resolution);

	//vec3 mean_color = vec3(0.0);
	//float mean_alpha = 0.0;
	//for (int i = 0; i < u_samples; i++) {
	//	vec4 sample_color = texelFetch(sampler, ivec2(gl_FragCoord.xy), i);
	//	mean_color += sample_color.a * sample_color.rgb;
	//}

	float alpha = 0.0;
	int n = 0;
	vec3 mean_color = vec3(0.0);
	for (int i = 0; i < u_samples; i++) {
		vec4 sample_color = texelFetch(sampler, ivec2(gl_FragCoord.xy), i);
		alpha += sample_color.a;
		if (sample_color.a > 0.0) {
			n += 1;
			mean_color += sample_color.a * sample_color.rgb;
		}
	}

	if (alpha > 0.0) {
		mean_color /= alpha;
	}

	// frag_color = texelFetch(sampler, ivec2(gl_FragCoord.xy), 0);
	frag_color = vec4(mean_color, alpha / float(u_samples));
	// frag_color = vec4(1.0, 1.0, 0.0, 1.0);
}