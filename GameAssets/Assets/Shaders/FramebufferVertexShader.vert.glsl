layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texture;

out vec2 v_position;
out vec2 v_texture;

void main() {
	gl_Position = vec4(a_position, 0.0, 1.0);
	v_position = a_position;
	v_texture = a_texture;
}