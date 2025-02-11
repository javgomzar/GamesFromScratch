#version 430 core

layout(points) in;
layout (line_strip, max_vertices = 6) out;

in vec3 v_normal[];

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;
uniform mat4 u_normal;

void main() {
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + u_projection * vec4(v_normal[0], 0);
    EmitVertex();
    
    EndPrimitive();
}