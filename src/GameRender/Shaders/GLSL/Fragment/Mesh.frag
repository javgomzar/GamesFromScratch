#version 450
precision highp float;

#ifdef VULKAN
layout (std140, set = 0, binding = 1) uniform LightUniforms 
#else
layout (std140, binding = 1) uniform LightUniforms
#endif
{
	vec3 direction;
	vec3 color;
	vec3 cameraPosition;
	float ambient;
	float diffuse;
} LightUBO;

#ifdef VULKAN
layout(std140, set = 1, binding = 0) uniform ColorUniform 
#else
layout(std140, binding = 2) uniform ColorUniform
#endif
{
	vec4 color;
} ColorUBO;

#ifdef VULKAN
layout (set = 2, binding = 0) uniform sampler2D binded_texture;
#else
layout (binding = 0) uniform sampler2D binded_texture;
#endif

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec2 v_texture;
layout (location = 2) in vec3 v_normal;
layout (location = 0) out vec4 frag_color;

void main() {
	vec4 texture_color = texture(binded_texture, v_texture);

	vec3 norm = normalize(v_normal);

	float diff = LightUBO.diffuse * max(dot(norm, -LightUBO.direction), 0);
	vec3 viewDirection = normalize(LightUBO.cameraPosition - v_position);
	vec3 reflectionDirection = reflect(LightUBO.direction, norm);
	float spec = pow(max(dot(viewDirection, reflectionDirection), 0.0), 8);

	vec3 out_color = (LightUBO.ambient + diff) * texture_color.rgb * ColorUBO.color.rgb + spec * LightUBO.color;

	frag_color = vec4(out_color, 1.0);
}
