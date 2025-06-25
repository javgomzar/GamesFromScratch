#version 450
#extension GL_KHR_vulkan_glsl : enable
precision highp float;

#ifdef VULKAN
layout (std140, set = 0, binding = 0) uniform GlobalUniforms
#else
layout (std140, binding = 0) uniform GlobalUniforms
#endif
{
	mat4 projection;
	mat4 view;
	vec2 resolution;
	float time;
} GlobalUBO;

#ifdef VULKAN
layout (std140, set = 1, binding = 4) uniform KernelUniforms 
#else
layout (std140, binding = 6) uniform KernelUniforms 
#endif
{
	mat3 kernel;
} KernelUBO;

#ifdef VULKAN
layout (set = 2, binding = 0) uniform sampler2D binded_texture;
#else
layout (binding = 0) uniform sampler2D binded_texture;
#endif

layout (location = 1) in vec2 v_texture;
layout (location = 0) out vec4 frag_color;

void main() {
	vec4 result = vec4(0);
	for (int y = -1; y <= 1; y++) {
		for (int x = -1; y <= 1; x++) {
			vec2 t = v_texture + vec2(x, y) / GlobalUBO.resolution;
			if (t.x >= 0. && t.y >= 0. && t.x <= 1. && t.y <= 1.) {
				vec4 color = texture(binded_texture, t);
				result += KernelUBO.kernel[y + 1][x + 1] * color;
			}
		}
	}

	frag_color = result;
}
