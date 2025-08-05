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
	float DPI;
    float Points;
} TextUBO;

layout(location = 0) in vec2 a_position;
layout(location = 0) out vec3 v_position;

void main() {
	vec2 result = (2 * vec2(a_position.x, a_position.y) / GlobalUBO.resolution) + vec2(-1.0, 1.0);
    result *= TextUBO.Points * TextUBO.DPI / 72.0;

	v_position = vec3(result, 0);
	gl_Position = vec4(result, 0, 1.0);
}