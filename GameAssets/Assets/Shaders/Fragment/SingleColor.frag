#version 450
precision highp float;

#ifdef VULKAN
layout(std140, set = 1, binding = 0) uniform ColorUniform 
#else
layout(std140, binding = 2) uniform ColorUniform
#endif
{
	vec4 color;
} ColorUBO;

layout (location = 0) out vec4 frag_color;

void main() {
	frag_color = ColorUBO.color;
}