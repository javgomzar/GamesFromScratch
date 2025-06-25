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

layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

layout (location = 0) in vec3 normal[];
layout (location = 0) out vec3 v_position;
layout (location = 1) out vec3 v_texture;
layout (location = 2) out vec3 v_normal;

void main() {

	gl_Position = gl_in[0].gl_Position + cos(GlobalUBO.time) * vec4(normal[0], 0.);
	EmitVertex();

	gl_Position = gl_in[1].gl_Position + cos(GlobalUBO.time) * vec4(normal[0], 0.);
	EmitVertex();

	gl_Position = gl_in[2].gl_Position + cos(GlobalUBO.time) * vec4(normal[0], 0.);
	EmitVertex();

	EndPrimitive();
}