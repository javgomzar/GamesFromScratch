#version 430
precision highp float;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texture;
layout(location = 2) in vec3 a_normal;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;
uniform mat4 u_normal;

out vec3 v_position;
out vec3 v_normal;
out vec2 v_texture;

void main() {
	v_position = a_position;
	v_texture = a_texture;
	v_normal = (u_view * normalize(u_normal * vec4(a_normal, 0))).xyz;
	gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0);
}