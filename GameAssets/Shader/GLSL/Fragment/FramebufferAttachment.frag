#version 450
precision highp float;

#ifdef VULKAN
layout(set = 2, binding = 0) uniform sampler2D binded_texture;
#else
layout(binding = 0) uniform sampler2D binded_texture;
#endif

#ifdef VULKAN
layout(set = 2, binding = 1) uniform sampler2D attachment_texture;
#else
layout(binding = 1) uniform sampler2D attachment_texture;
#endif

layout (location = 1) in vec2 v_texture;
layout (location = 0) out vec4 frag_color;

void main() {
	vec4 texture_color = texture(binded_texture, v_texture);

	frag_color = texture_color;
	gl_FragDepth = texture(attachment_texture, v_texture).r;
}
