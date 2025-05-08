#version 450
#extension GL_KHR_vulkan_glsl : enable
precision highp float;

#ifdef VULKAN
layout (std140, set = 0, binding = 3) uniform AntialiasingUniforms 
#else
layout (std140, binding = 10) uniform AntialiasingUniforms
#endif
{
	int samples;
} AntialiasingUBO;

#ifdef VULKAN
layout(set = 2, binding = 0) uniform sampler2DMS color_sampler;
#else
layout(binding = 0) uniform sampler2DMS color_sampler;
#endif

#ifdef VULKAN
layout(set = 2, binding = 1) uniform sampler2DMS attachment_sampler;
#else
layout(binding = 1) uniform sampler2DMS attachment_sampler;
#endif

layout (location = 1) in vec2 v_texture;
layout (location = 0) out vec4 frag_color;

void main() {
	float alpha = 0.0;
	vec3 mean_color = vec3(0.0);
	float min_depth = 1.0;
	for (int i = 0; i < AntialiasingUBO.samples; i++) {
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

	frag_color = vec4(mean_color, alpha / float(AntialiasingUBO.samples));
	gl_FragDepth = min_depth;
}