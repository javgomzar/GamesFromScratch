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
	float ambient;
	float diffuse;
} LightUBO;

#ifdef VULKAN
layout (std140, set = 1, binding = 0) uniform ColorUniform 
#else
layout (std140, binding = 2) uniform ColorUniform 
#endif
{
	vec4 color;
} ColorUBO;

layout (location = 0) in vec3 v_position;
layout (location = 0) out vec4 frag_color;

void main() {
	float diffuse = LightUBO.diffuse * max(dot(normalize(v_position), -LightUBO.direction), 0.0);
	frag_color = ColorUBO.color * vec4((LightUBO.ambient + diffuse) * LightUBO.color, 1.0);
}