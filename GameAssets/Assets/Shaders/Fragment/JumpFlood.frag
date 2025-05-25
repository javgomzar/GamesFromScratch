#version 450
#extension GL_KHR_vulkan_glsl : enable
precision highp float;

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
layout(set = 2, binding = 0) uniform sampler2D binded_texture;
#else
layout(binding = 0) uniform sampler2D binded_texture;
#endif
//layout(binding = 1) uniform sampler2D attachment_texture;

const int maxSteps = 12;

#ifdef VULKAN
layout (std140, set = 1, binding = 4) uniform OutlineUniforms 
#else
layout (std140, binding = 5) uniform OutlineUniforms
#endif
{
    float width;
    int level;
} OutlineUBO;

layout (location = 1) in vec2 v_texture;
layout (location = 0) out vec4 frag_color;

vec3 StepJFA(vec2 p) {
    vec3 closest = vec3(-1.0, 0.0, 0.0);
    double closest_distance = 9999999.0;
    float w = OutlineUBO.level;
    for (int y = -1; y <= 1; ++y) {
    for (int x = -1; x <= 1; ++x) {
        vec2 t = p + w * vec2(x,y);
        if (t.x >= 0. && t.y >= 0. && t.x <= GlobalUBO.resolution.x && t.y <= GlobalUBO.resolution.y) {
            vec4 stored_color = texture(binded_texture, t / GlobalUBO.resolution);
            if (stored_color.b > 0.) { // skip computing for yet invalid seeds
                vec2 stored_position = stored_color.rg;
                double d = distance(p, stored_position);
                if (closest_distance > d) {
                    closest_distance = d;
                    closest = stored_color.rgb;
                }
            }
        }
    }}

    return closest;
}

void main() {
    vec3 closest = StepJFA(gl_FragCoord.xy);
    if (closest.x < 0.) {
        frag_color = texture(binded_texture, v_texture);
        //gl_FragDepth = texture(attachment_texture, v_texture).r;
    }
    else {
        frag_color = vec4(closest, 1.0);
        //gl_FragDepth = closest.b;
    }
}