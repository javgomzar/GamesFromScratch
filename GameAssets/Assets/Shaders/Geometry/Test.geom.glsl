#version 430
precision highp float;

uniform float u_time;

layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

in VS_OUT {
	vec3 position;
	vec3 normal;
	vec2 texture;
} vs_out;

out vec3 v_position;
out	vec3 v_normal;
out	vec2 v_texture;

void main() {

	gl_Position = gl_in[0].gl_Position + cos(u_time) * vec4(vs_out.normal, 0.);
	EmitVertex();

	gl_Position = gl_in[1].gl_Position + cos(u_time) * vec4(vs_out.normal, 0.);
	EmitVertex();

	gl_Position = gl_in[2].gl_Position + cos(u_time) * vec4(vs_out.normal, 0.);
	EmitVertex();

	EndPrimitive();
}