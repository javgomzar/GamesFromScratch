layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texture;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 v_position;
out vec3 v_normal;
out vec2 v_texture;

void main() {
	vec4 result = projection * view * model * vec4(a_position, 1.0);
	v_position = a_position;
	gl_Position = result;
	v_normal = (model * vec4(normalize(a_normal), 0)).xyz;
	v_texture = a_texture;
}