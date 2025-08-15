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

layout (location = 0) in vec2 barycentric;
layout (location = 0) out vec4 frag_color;

void main() {
	float f = 0.5 * barycentric.x + barycentric.y;
    if (f * f < barycentric.y) {
        frag_color = ColorUBO.color;
    }
    else {
        frag_color = vec4(0);
    }
}