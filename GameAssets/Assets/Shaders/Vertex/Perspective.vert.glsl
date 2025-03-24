#version 430
precision highp float;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texture;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in ivec2 bone_ids;
layout(location = 4) in vec2 bone_weights;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;
uniform mat4 u_normal;

uniform int nBones;
const int MAX_BONES = 32;
uniform mat4 bone_transforms[MAX_BONES];
uniform mat4 bone_normal_transforms[MAX_BONES];

out vec3 v_position;
out vec3 v_normal;
out vec2 v_texture;

void main() {
	v_position = a_position;
	v_texture = a_texture;
	v_normal = (normalize(u_normal * vec4(a_normal, 0))).xyz;
	if (nBones > 0) {
		// Positions
		vec4 first_bone = bone_transforms[bone_ids[0]] * vec4(a_position, 1.0);
		vec4 second_bone = bone_transforms[bone_ids[1]] * vec4(a_position, 1.0);
		v_position = (bone_weights[0] * first_bone + bone_weights[1] * second_bone).xyz;

		// Normals
		vec4 first_bone_normal = bone_normal_transforms[bone_ids[0]] * vec4(a_normal, 0.0);
		vec4 second_bone_normal = bone_normal_transforms[bone_ids[1]] * vec4(a_normal, 0.0);
		v_normal = normalize(u_normal * (bone_weights[0] * first_bone_normal + bone_weights[1] * second_bone_normal)).xyz;
	}
	gl_Position = u_projection * u_view * u_model * vec4(v_position, 1.0);
}