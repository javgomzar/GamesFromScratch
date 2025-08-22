#version 450

/*
    Tessellation primitive generation
        - `equal_spacing`: Subdivisions of equal sizes                  .__.__.__.__.
        - `fractional_odd_spacing`: Odd number of subdivisions          .__._____.__.
        - `fractional_even_spacing`: Even number of subdivisions        ._.___.___._.
*/
layout (isolines, equal_spacing, ccw) in;

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
layout(std140, set = 2, binding = 2) uniform TextUniforms 
#else 
layout(std140, binding = 8) uniform TextUniforms 
#endif
{
    vec2 Pen;
	float Size;
} TextUBO;

layout (location = 0) out float t;

void main() {
    t = gl_TessCoord.x;

    // Control point position coordinates
    vec2 p0 = gl_in[0].gl_Position.xy;
    vec2 p1 = gl_in[1].gl_Position.xy;
    vec2 p2 = gl_in[2].gl_Position.xy;

    // Quadratic interpolation
    float u = 1.0 - t; // t backwards
    vec2 interpolation = u * u * p0 + 2.0 * t * u * p1 + t * t * p2;

    vec2 normalized_pen = (2 * vec2(TextUBO.Pen.x, -TextUBO.Pen.y) / GlobalUBO.resolution) + vec2(-1.0, 1.0);

    gl_Position = vec4(normalized_pen + TextUBO.Size * interpolation, 0.0, 1.0);
}
