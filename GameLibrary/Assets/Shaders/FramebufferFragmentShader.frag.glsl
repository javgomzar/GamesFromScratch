
uniform sampler2D binded_texture;
uniform float Alpha;

out vec4 FragColor;

in vec2 v_texture;

void main() {
	vec4 TextureColor = texture(binded_texture, v_texture);
	TextureColor.xyz = vec3(TextureColor.x + TextureColor.y + TextureColor.z) / 3.0;
	FragColor = TextureColor * vec4(1.0, 1.0, 1.0, Alpha);
}
