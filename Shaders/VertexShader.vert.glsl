layout(location = 0) in vec4 position;

out vec4 v_color;

uniform mat4 projection;
uniform mat4 modelview;

void main() {
	gl_Position = projection * modelview * vec4(position.x, position.y, position.z, 1.0);
	float intensity = 0.6 + 0.4 * dot(normalize(position.xyz), normalize(vec3(1.0, 1.0, 1.0)));
	v_color = vec4(intensity*vec3(0.25, 0.26, 0.7), 1.0);
}