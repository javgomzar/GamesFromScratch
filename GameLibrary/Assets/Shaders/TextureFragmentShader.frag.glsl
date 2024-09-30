
uniform sampler2D binded_texture;

in vec3 v_normal;
in vec2 v_texture;

uniform vec3 light_direction;
uniform vec3 light_color;
uniform float light_ambient;
uniform float light_diffuse;

out vec4 FragColor;

void main() {
	// FragColor = texture(binded_texture, v_texture);
	float diffuse = light_diffuse * max(dot(v_normal, -light_direction), 0.0);
	// FragColor = vec4((light_ambient + diffuse) * light_color, 1.0);
	vec4 TextureColor = texture(binded_texture, v_texture) * vec4(light_color, 1.0);
	FragColor = vec4((light_ambient + diffuse) * TextureColor.xyz, 1.0);
}