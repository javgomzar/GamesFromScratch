#version 450

/*
    Tessellation primitive generation
        - `equal_spacing`: Subdivisions of equal sizes                  .__.__.__.__.
        - `fractional_odd_spacing`: Odd number of subdivisions          .__._____.__.
        - `fractional_even_spacing`: Even number of subdivisions        ._.___.___._.
*/
layout (quads, equal_spacing, ccw) in;

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

layout (location = 0) out float t;

void main() {
    t = gl_TessCoord.x;

    // Control point position coordinates
    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;

    // Quadratic interpolation
    float u = 1.0 - t; // t backwards
    gl_Position = u * u * p0 + 2.0 * t * u * p1 + t * t * p2;
}
