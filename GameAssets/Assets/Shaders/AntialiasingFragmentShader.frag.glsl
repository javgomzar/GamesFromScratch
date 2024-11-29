layout(binding = 0) uniform sampler2DMS sampler;

uniform int u_samples;
uniform vec2 u_resolution;

in vec2 v_texture;

out vec4 frag_color;

void main() {
	float alpha = 0.0;
	vec3 mean_color = vec3(0.0);
	for (int i = 0; i < u_samples; i++) {
		vec4 sample_color = texelFetch(sampler, ivec2(gl_FragCoord.xy), i);
		alpha += sample_color.a;
		if (sample_color.a > 0.0) {
			mean_color += sample_color.a * sample_color.rgb;
		}
	}

	if (alpha > 0.0) {
		mean_color /= alpha;
	}

	frag_color = vec4(mean_color, alpha / float(u_samples));
}