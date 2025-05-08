#version 450
#extension GL_KHR_vulkan_glsl : enable

/*
    Tessellation primitive generation
        - `equal_spacing`: Subdivisions of equal sizes                  .__.__.__.__.
        - `fractional_odd_spacing`: Odd number of subdivisions          .__._____.__.
        - `fractional_even_spacing`: Even number of subdivisions        ._.___.___._.
*/
layout (quads, equal_spacing, ccw) in;

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
layout (set = 2, binding = 0) uniform sampler2D heightMap;
#else
layout (binding = 0) uniform sampler2D heightMap;
#endif

layout (location = 0) in vec2 texture_coord[];
layout (location = 0) out float height;

void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // Control point texture coordinates
    vec2 t00 = texture_coord[0];
    vec2 t01 = texture_coord[1];
    vec2 t10 = texture_coord[2];
    vec2 t11 = texture_coord[3];

    // Bilinear interpolation for texture coordinates
    vec2 t0       = t01 * u + (1.0 - u) * t00;
    vec2 t1       = t11 * u + (1.0 - u) * t10;
    vec2 texCoord =  t1 * v + (1.0 - v) * t0;

    height = 0.2 * texture(heightMap, texCoord).y;
    
    // Control point position coordinates
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    // Compute normal
    vec4 uVec = p01 - p00;
    vec4 vVec = p10 - p00;
    //vec4 normal = normalize(vec4(cross(uVec.xyz, vVec.xyz), 0.0));
    vec4 normal = vec4(0.0, 1.0, 0.0, 0.0);

    // Bilinear interpolation for position coordinates
    vec4 p0 = p01 * u + (1.0 - u) * p00;
    vec4 p1 = p11 * u + (1.0 - u) * p10;
    vec4 p  =  p1 * v + (1.0 - v) * p0;

    // Height
    p += normal * height;

    gl_Position = ProjectionUBO.world_projection * ProjectionUBO.view * ModelUBO.model * p;
}

