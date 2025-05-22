#version 450

layout (points) in;
layout (line_strip, max_vertices = 2) out;

layout (location = 2) in vec3 v_normal[];

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
} ProjectionUBO;

void main() {
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + ProjectionUBO.projection * ProjectionUBO.view * vec4(v_normal[0], 0);
    EmitVertex();
    
    EndPrimitive();
}