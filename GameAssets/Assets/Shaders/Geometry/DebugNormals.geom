#version 450

layout (points) in;
layout (line_strip, max_vertices = 2) out;

layout (location = 2) in vec3 v_normal[];

#ifdef VULKAN
layout(std140, set = 0, binding = 1) uniform ProjectionUniforms 
#else
layout(std140, binding = 1) uniform ProjectionUniforms
#endif
{
	mat4 world_projection;
	mat4 screen_projection;
	mat4 view;
} ProjectionUBO;

void main() {
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + ProjectionUBO.world_projection * ProjectionUBO.view * vec4(v_normal[0], 0);
    EmitVertex();
    
    EndPrimitive();
}