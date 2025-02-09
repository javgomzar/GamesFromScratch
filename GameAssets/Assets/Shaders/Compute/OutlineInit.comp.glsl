#version 430
precision highp float;

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) readonly uniform image2D source;
layout(rgba32f, binding = 0) writeonly uniform image2D target;
//layout(binding = 1) uniform sampler2D attachment_texture;

uniform vec2 u_resolution;

void main() {
	ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
	vec4 image_color = imageLoad(source, texelCoord);

	//float depth = texture(attachment_texture, texelCoord / u_resolution).r + 0.005;

	vec4 result = vec4(0.);
	if (image_color.r > 0.99) {
		result = vec4(gl_GlobalInvocationID.xy, 1., 1.0);
	}
	if (image_color.r < 0.01) {
		result = vec4(-1., 0., 0., 1.);
	}
	else {
		mat3 sobel_x = mat3(
			vec3(-1., 0., 1.), 
			vec3(-2., 0., 2.),
			vec3(-1., 0., 1.)
		);
		mat3 sobel_y = mat3(
			vec3(-1.,-2.,-1.),
			vec3( 0., 0., 0.),
			vec3( 1., 2., 1.)
		);

		float offset_x = 0;
		float offset_y = 0;

		for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < 3; ++i) {
			vec2 p = (gl_GlobalInvocationID.xy + vec2(i-1,j-1)) / u_resolution.xy;
			float value = image_color.r;
			offset_x += sobel_x[j][i] * value;
			offset_y += sobel_y[j][i] * value;
		}}

		if (abs(offset_x) <= 0.005 && abs(offset_y) < 0.005) {
			result = vec4(gl_GlobalInvocationID.xy, .1, 1.0);
		}
		else {
			vec2 offset = (1. - image_color.r) * normalize(vec2(offset_x, offset_y));
			result = vec4(gl_GlobalInvocationID.xy + offset, .1, 1.);
		}
	}

	imageStore(target, texelCoord, result);
}
