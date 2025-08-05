#version 450
precision highp float;

#if VULKAN
layout (std140, set = 0, binding = 1) uniform LightUniforms 
#else
layout (std140, binding = 1) uniform LightUniforms
#endif
{
    vec3 direction;
    vec3 color;
    float ambient;
    float diffuse;
} UBO;

layout (location = 0) in vec3 v_normal;
layout (location = 1) in float height;
layout (location = 0) out vec4 frag_color;

void main() {
    vec4 blue = vec4(0.04, 0.22, 0.25, 1.0);
    vec4 white = vec4(0.4706, 0.6314, 0.7922, 1.0);
    frag_color = mix(blue, white, (height + 0.5));
}