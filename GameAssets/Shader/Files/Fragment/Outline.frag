#version 450
precision highp float;

#ifdef VULKAN
layout(set = 2, binding = 0) uniform sampler2D binded_texture;
#else
layout(binding = 0) uniform sampler2D binded_texture;
#endif
//layout(binding = 1) uniform sampler2D attachment_texture;

#ifdef VULKAN
layout(std140, set = 0, binding = 0) uniform GlobalUniforms 
#else 
layout(std140, binding = 0) uniform GlobalUniforms 
#endif
{
	mat4 projection;
	mat4 view;
	vec2 resolution;
	float time;
} GlobalUBO;

#ifdef VULKAN
layout (std140, set = 1, binding = 0) uniform ColorUniforms 
#else
layout (std140, binding = 2) uniform ColorUniforms 
#endif
{
	vec4 color;
} ColorUBO;

#ifdef VULKAN
layout (std140, set = 1, binding = 6) uniform OutlineUniforms 
#else
layout (std140, binding = 5) uniform OutlineUniforms
#endif
{
	float width;
	int Level;
} OutlineUBO;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

layout (location = 1) in vec2 v_texture;
layout (location = 0) out vec4 frag_color;

float triangle(float x) {
	float result = abs(fract(x) * 2.0 - 1.0);
	return result*result*result*result;
}

void main() {
	vec4 texture_color = texture(binded_texture, v_texture);

	float width = OutlineUBO.width;

	if (texture_color.b > 0.) {
		float d = distance(gl_FragCoord.xy, texture_color.rg);

		if (d >= width) {
			frag_color = vec4(0., 0., 0., 0.);
			//gl_FragDepth = 1.0;
		}
		else {
			vec3 color = ColorUBO.color.rgb;
			//float hue = fract(0.1*d + 0.5*GlobalUBO.time);
			//vec3 color = hsv2rgb(vec3(hue, 1., 1.));
			double alpha = ColorUBO.color.a;
			if (d >= width - 1.) {
				alpha *= (width - d);
				//gl_FragDepth = texture_color.b;
			}
			frag_color = vec4(alpha*color, alpha);
			//gl_FragDepth = 1.0;
		}
	}
	else {
		frag_color = vec4(0., 0., 0., 0.);
		//gl_FragDepth = 1.0;
	}
}