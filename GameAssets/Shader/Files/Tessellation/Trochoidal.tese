#version 450
#extension GL_KHR_vulkan_glsl : enable

/*
    Tessellation primitive generation
        - `equal_spacing`: Subdivisions of equal sizes                  .__.__.__.__.
        - `fractional_odd_spacing`: Odd number of subdivisions          .__._____.__.
        - `fractional_even_spacing`: Even number of subdivisions        ._.___.___._.
*/
layout (quads, equal_spacing, ccw) in;

#define TAU 6.283185307179586476925287
#define g 9.80665
#define L 0.2

#ifdef VULKAN
layout(std140, set = 0, binding = 0) uniform GlobalUniforms 
#else
layout (std140, binding = 0) uniform GlobalUniforms
#endif
{
    mat4 projection;
    mat4 view;
    vec2 resolution;
    float time;
} GlobalUBO;

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
layout(set = 2, binding = 0) uniform sampler2D heightMap;
#else
layout(binding = 0) uniform sampler2D heightMap;
#endif

layout(location = 0) in vec2 texture_coord[];
layout(location = 0) out vec3 v_normal;
layout(location = 1) out float height;

float rand (in vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float square(float x) {
    return x*x;
}

mat2 m2 = mat2(1.6,-1.2,1.2,1.6);

float fbm( vec2 p ) {
  float f = 0.5000*rand( p ); p = m2*p;
  f += 0.2500*rand( p ); p = m2*p;
  f += 0.1666*rand( p ); p = m2*p;
  f += 0.0834*rand( p );
  return f;
}

float spectrum(float k) {
    return exp(-1.0/(square(L*k))) / square(square(k));
}

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
    
    // Control point position coordinates
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    // Compute normal
    vec4 uVec = p01 - p00;
    vec4 vVec = p10 - p00;
    //vec4 normal = normalize(vec4(cross(uVec.xyz, vVec.xyz), 0.0));
    v_normal = vec3(0.0, 1.0, 0.0);

    // Bilinear interpolation for position coordinates
    vec4 p0 = p01 * u + (1.0 - u) * p00;
    vec4 p1 = p11 * u + (1.0 - u) * p10;
    vec4 p  =  p1 * v + (1.0 - v) * p0;

    // Height
    vec3 wind = vec3(1.0, 0.0, 1.0);
    float wind_speed = length(wind);
    float x = 0;
	float y = 0;
    float z = 0;
    for (int i = 0; i < 5; i++) {
        float kx = (1.0 - 2.0 * rand(vec2(i,0)));
        float kz = (1.0 - 2.0 * rand(vec2(0,i)));
        float k = length(vec2(kx,kz));
        float w = sqrt(g*k);
        y += (1 - abs(sin(kx*p.x + kz*p.z - w*GlobalUBO.time)));
    }
    height = y;
    p += vec4(x,y,z,0);

    gl_Position = GlobalUBO.projection * GlobalUBO.view * ModelUBO.model * p;
}