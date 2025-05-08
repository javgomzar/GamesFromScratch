#version 450
precision highp float;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texture;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in ivec2 bone_ids;
layout(location = 4) in vec2 bone_weights;

const int MAX_BONES = 32;

#ifdef VULKAN
layout(std140, set = 0, binding = 0) uniform GlobalUniforms 
#else 
layout(std140, binding = 0) uniform GlobalUniforms 
#endif
{
	vec2 resolution;
	float time;
} GlobalUBO;

#ifdef VULKAN
layout(std140, set = 0, binding = 1) uniform ProjectionUniforms 
#else 
layout(std140, binding = 1) uniform ProjectionUniforms 
#endif
{
	mat4 world_projection;
	mat4 screen_projection;
	mat4 view;
} ProjectionUBO;

#ifdef VULKAN
layout(std140, set = 1, binding = 0) uniform ScreenUniforms 
#else 
layout(std140, binding = 2) uniform ScreenUniforms 
#endif
{
	int use_screen_projection;
} ScreenUBO;

#ifdef VULKAN
layout(std140, set = 1, binding = 1) uniform ModelUniforms 
#else 
layout(std140, binding = 3) uniform ModelUniforms 
#endif
{
	mat4 model;
	mat4 normal;
} ModelUBO;

#ifdef VULKAN
layout(std140, set = 1, binding = 3) uniform BoneUniforms 
#else 
layout(std140, binding = 4) uniform BoneUniforms 
#endif
{
	mat4 bone_transforms[MAX_BONES];
	mat4 bone_normal_transforms[MAX_BONES];
	int nBones;
} BonesUBO;

layout(location = 0) out vec3 v_position;
layout(location = 1) out vec2 v_texture;
layout(location = 2) out vec3 v_normal;

void main() {
	v_position = a_position;
	v_texture = a_texture;
	v_normal = (normalize(ModelUBO.normal * vec4(a_normal, 0))).xyz;

	mat4 projection = ScreenUBO.use_screen_projection != 0 ? ProjectionUBO.screen_projection : ProjectionUBO.world_projection;
	mat4 view = ScreenUBO.use_screen_projection != 0 ? mat4(1.0) : ProjectionUBO.view;
	mat4 model = ScreenUBO.use_screen_projection != 0 ? mat4(1.0) : ModelUBO.model;

	if (BonesUBO.nBones > 0) {
		// Positions
		vec4 first_bone = BonesUBO.bone_transforms[bone_ids[0]] * vec4(a_position, 1.0);
		vec4 second_bone = BonesUBO.bone_transforms[bone_ids[1]] * vec4(a_position, 1.0);
		v_position = (bone_weights[0] * first_bone + bone_weights[1] * second_bone).xyz;

		// Normals
		vec4 first_bone_normal = BonesUBO.bone_normal_transforms[bone_ids[0]] * vec4(a_normal, 0.0);
		vec4 second_bone_normal = BonesUBO.bone_normal_transforms[bone_ids[1]] * vec4(a_normal, 0.0);
		v_normal = normalize(ModelUBO.normal * (bone_weights[0] * first_bone_normal + 
										        bone_weights[1] * second_bone_normal)).xyz;
	}
	gl_Position = projection * view * model * vec4(v_position, 1.0);
}