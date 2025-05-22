#version 450

#ifdef VULKAN
layout (std140, set = 1, binding = 0) uniform ColorUniform
#else
layout (std140, binding = 2) uniform ColorUniform
#endif
{
	vec4 color;
} ColorUBO;

#if VULKAN
layout (set = 2, binding = 0) uniform sampler2D binded_texture;
#else
layout (binding = 0) uniform sampler2D binded_texture;
#endif

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec2 v_texture;
layout (location = 0) out vec4 frag_color;

void main() {
	frag_color = ColorUBO.color * texture(binded_texture, v_texture);
}