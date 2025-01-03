
uniform sampler2D binded_texture;

uniform vec3 light_direction;
uniform vec3 light_color;
uniform float light_ambient;
uniform float light_diffuse;

out vec4 frag_color;

in vec2 v_texture;
in vec3 v_normal;

void main() {
	vec4 texture_color = texture(binded_texture, v_texture);

	float diff = light_diffuse * max(dot(v_normal, -light_direction), 0);
	vec3 out_color = (light_ambient + diff) * light_color * texture_color.rgb;

	frag_color = vec4(out_color, 1.0);
}
