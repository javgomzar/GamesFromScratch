
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

	// Inverted
	// texture_color.xyz = vec3(1.0) - texture_color.xyz;

	//	Black and white
	// texture_color.xyz = vec3(texture_color.x + texture_color.y + texture_color.z) / 6.0;

	float diff = light_diffuse * max(dot(v_normal, -light_direction), 0);
	vec3 out_color = (light_ambient + diff) * light_color * texture_color.rgb;

	// Old computer
	// frag_color = texture_color * vec4(0.5, 1.0, 0.5, Alpha);

	frag_color = vec4(out_color, 1.0);
}
