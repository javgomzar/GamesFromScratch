#version 450
precision highp float;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texture;
layout(location = 2) in vec3 a_normal;

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
layout(std140, set = 1, binding = 1) uniform ModelUniforms 
#else 
layout(std140, binding = 3) uniform ModelUniforms 
#endif
{
	mat4 model;
	mat4 normal;
} ModelUBO;

layout(location = 0) out vec3 v_position;
layout(location = 1) out vec2 v_texture;
layout(location = 2) out vec3 v_normal;

void main() {
	v_position = a_position;
	v_texture = a_texture;
	v_normal = normalize((ModelUBO.normal * vec4(a_normal, 0)).xyz);

	gl_Position = GlobalUBO.projection * GlobalUBO.view * ModelUBO.model * vec4(v_position, 1.0);
	gl_PointSize = 50.0f;
}