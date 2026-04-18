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
    vec2 mouse;
    vec2 lastmouse;
	float time;
} GlobalUBO;

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_barycentric;
layout(location = 2) in vec3 a_text;
layout(location = 3) in vec4 a_color;

layout(location = 0) out vec2 barycentric;
layout(location = 1) out vec4 color;

void main() {
    barycentric = a_barycentric;
	color = a_color;

	vec2 sized = a_text.xy + a_text.z * a_position;
    vec2 result = (2 * vec2(sized.x, -sized.y) / GlobalUBO.resolution) + vec2(-1, 1);

	gl_Position = vec4(result, 0, 1);
	gl_PointSize = 10;
}