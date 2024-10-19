
uniform sampler2D binded_texture;

out vec4 FragColor;

in vec2 v_texture;

void main() {
	vec4 TextureColor = texture(binded_texture, v_texture);

	// Inverted
	// TextureColor.xyz = vec3(1.0) - TextureColor.xyz;

	//	Black and white
	// TextureColor.xyz = vec3(TextureColor.x + TextureColor.y + TextureColor.z) / 6.0;

	FragColor = TextureColor;

	// Old computer
	// FragColor = TextureColor * vec4(0.5, 1.0, 0.5, Alpha);
}
