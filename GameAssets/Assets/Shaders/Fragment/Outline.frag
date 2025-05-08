#version 450
#extension GL_KHR_vulkan_glsl : enable
precision highp float;

#ifdef VULKAN
layout(set = 2, binding = 0) uniform sampler2D binded_texture;
#else
layout(binding = 0) uniform sampler2D binded_texture;
#endif
//layout(binding = 1) uniform sampler2D attachment_texture;

#ifdef VULKAN
layout (std140, set = 1, binding = 2) uniform ColorUniforms 
#else
layout (std140, binding = 5) uniform ColorUniforms 
#endif
{
	vec4 color;
} ColorUBO;

#ifdef VULKAN
layout (std140, set = 1, binding = 6) uniform OutlineUniforms 
#else
layout (std140, binding = 7) uniform OutlineUniforms
#endif
{
	float width;
} OutlineUBO;

layout (location = 1) in vec2 v_texture;
layout (location = 0) out vec4 frag_color;

void main() {
	vec4 texture_color = texture(binded_texture, v_texture);

	if (texture_color.b > 0.) {
		double d = distance(gl_FragCoord.xy, texture_color.rg);

		if (d < OutlineUBO.width - 1.) {
			frag_color = ColorUBO.color;
			//gl_FragDepth = texture_color.b;
		}
		else {
			if (d < OutlineUBO.width) {
				frag_color = vec4(ColorUBO.color.rgb, (OutlineUBO.width - d) * ColorUBO.color.a);
				//gl_FragDepth = texture_color.b;
			}
			else { 
				frag_color = vec4(1.0, 0.0, 1.0, 0.0);
				//gl_FragDepth = 1.0;
			}
		}
	}
	else {
		frag_color = vec4(1., 0., 1., 0.);
		//gl_FragDepth = 1.0;
	}
}