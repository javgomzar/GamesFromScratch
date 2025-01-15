#version 430
precision highp float;

layout(location = 0) in vec3 a_position;
//layout(location = 1) in vec2 a_texture;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

out vec3 v_position;
//out vec2 v_texture;
out vec3 v_normal;

void main() {
	vec4 result = u_projection * u_view * u_model * vec4(a_position, 1.0);
	v_position = a_position;
	gl_Position = result;
	//v_texture = a_texture;
	v_normal = vec3(1.0);
}