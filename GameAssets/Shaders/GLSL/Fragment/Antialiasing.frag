#version 450
precision highp float;

#ifdef VULKAN
layout (std140, set = 0, binding = 2) uniform AntialiasingUniforms 
#else
layout (std140, binding = 7) uniform AntialiasingUniforms
#endif
{
	int samples;
} AntialiasingUBO;

#ifdef VULKAN
layout(set = 2, binding = 0) uniform sampler2DMS binded_texture;
#else
layout(binding = 0) uniform sampler2DMS binded_texture;
#endif

#ifdef VULKAN
layout(set = 2, binding = 1) uniform sampler2DMS attachment_texture;
#else
layout(binding = 1) uniform sampler2DMS attachment_texture;
#endif

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec2 v_texture;
layout (location = 0) out vec4 frag_color;

void main() {
	vec4 mean_color = vec4(0.0);
	float min_depth = 1.0;
	for (int i = 0; i < AntialiasingUBO.samples; i++) {
		vec4 sample_color = texelFetch(binded_texture, ivec2(gl_FragCoord.xy), i);
		float sample_depth = texelFetch(attachment_texture, ivec2(gl_FragCoord.xy), i).r;
		mean_color += sample_color.a * sample_color;
		if (sample_depth < min_depth) {
			min_depth = sample_depth;
		}
	}

	frag_color = mean_color / float(AntialiasingUBO.samples);
	gl_FragDepth = min_depth;
}