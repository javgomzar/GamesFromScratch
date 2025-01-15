#version 430
precision highp float;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texture;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

out vec3 v_position;
out vec3 v_normal;
out vec2 v_texture;

void main() {
	vec4 result = u_projection * u_view * u_model * vec4(a_position, 1.0);
	v_position = a_position;
	gl_Position = result;
	v_normal = (u_model * vec4(normalize(a_normal), 0)).xyz;
	v_texture = a_texture;
}