#version 450
precision highp float;

layout(location = 0) in vec3 a_position;

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

layout(location = 0) out vec3 v_position;

void main() {
	v_position = a_position;

    mat4 view = GlobalUBO.view;
    view[3] = vec4(0, 0, 0, 1);

	vec4 Result = GlobalUBO.projection * view * vec4(v_position, 1.0);
    Result.z = 0.99999f * Result.w;
    gl_Position = Result;
}