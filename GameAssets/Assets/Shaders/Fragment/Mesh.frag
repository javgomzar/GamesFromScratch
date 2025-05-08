#version 450
#extension GL_KHR_vulkan_glsl : enable
precision highp float;

#ifdef VULKAN
layout (std140, set = 0, binding = 2) uniform LightUniforms 
#else
layout (std140, binding = 6) uniform LightUniforms
#endif
{
	vec3 direction;
	vec3 color;
	float ambient;
	float diffuse;
} LightUBO;

#ifdef VULKAN
layout (set = 2, binding = 0) uniform sampler2D binded_texture;
#else
layout (binding = 0) uniform sampler2D binded_texture;
#endif

layout (location = 1) in vec2 v_texture;
layout (location = 2) in vec3 v_normal;
layout (location = 0) out vec4 frag_color;

void main() {
	vec4 texture_color = texture(binded_texture, v_texture);

	float diff = LightUBO.diffuse * max(dot(v_normal, -LightUBO.direction), 0);
	vec3 out_color = (LightUBO.ambient + diff) * LightUBO.color * texture_color.rgb;

	frag_color = vec4(out_color, 1.0);
}
