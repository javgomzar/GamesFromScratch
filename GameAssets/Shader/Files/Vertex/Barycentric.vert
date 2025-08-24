#version 450
precision highp float;

#ifdef VULKAN
layout(std140, set = 0, binding = 0) uniform GlobalUniforms 
#else 
layout(std140, binding = 0) uniform GlobalUniforms 
#endif
{
	mat4 projection;
	mat4 view;
	vec2 resolution;
	float time;
} GlobalUBO;

#ifdef VULKAN
layout(std140, set = 2, binding = 2) uniform TextUniforms 
#else 
layout(std140, binding = 8) uniform TextUniforms 
#endif
{
    vec2 Pen;
	float Size;
} TextUBO;

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_barycentric;

layout(location = 0) out vec2 barycentric;

void main() {
    barycentric = a_barycentric;

	vec2 sized = TextUBO.Pen + TextUBO.Size * a_position;
    vec2 result = (2 * vec2(sized.x, -sized.y) / GlobalUBO.resolution) + vec2(-1.0, 1.0);

	gl_Position = vec4(result, 0, 1.0);
	gl_PointSize = 10.0f;
}