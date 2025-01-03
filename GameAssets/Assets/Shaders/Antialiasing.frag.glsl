layout(binding = 0) uniform sampler2DMS color_sampler;
layout(binding = 1) uniform sampler2DMS attachment_sampler;

uniform int u_samples;

in vec2 v_texture;

out vec4 frag_color;

void main() {
	float alpha = 0.0;
	vec3 mean_color = vec3(0.0);
	float min_depth = 1.0;
	for (int i = 0; i < u_samples; i++) {
		vec4 sample_color = texelFetch(color_sampler, ivec2(gl_FragCoord.xy), i);
		float sample_depth = texelFetch(attachment_sampler, ivec2(gl_FragCoord.xy), i).r;
		alpha += sample_color.a;
		if (sample_color.a > 0.0) {
			mean_color += sample_color.a * sample_color.rgb;
		}
		if (sample_depth < min_depth) {
			min_depth = sample_depth;
		}
	}

	if (alpha > 0.0) {
		mean_color /= alpha;
	}

	frag_color = vec4(mean_color, alpha / float(u_samples));
	gl_FragDepth = min_depth;
}