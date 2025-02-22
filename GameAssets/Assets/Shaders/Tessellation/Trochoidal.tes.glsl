#version 430 core

/*
    Tessellation primitive generation
        - `equal_spacing`: Subdivisions of equal sizes                  .__.__.__.__.
        - `fractional_odd_spacing`: Odd number of subdivisions          .__._____.__.
        - `fractional_even_spacing`: Even number of subdivisions        ._.___.___._.
*/
layout (quads, equal_spacing, ccw) in;

#define TAU 6.283185307179586476925287
#define g 9.80665

uniform sampler2D heightMap;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

uniform float u_time;

in vec2 texture_coord[];

out vec3 v_normal;
out float height;

float rand (in vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float square(float x) {
    return x*x;
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
    vec2 slope = vec2(0);
    float phi1 = 0;
    float k_squared = 0;
    float w_dot = 0;
	for(float kx = 0.0; kx < 16.1; kx += 0.5) {
        phi1 = kx * p.x;
        k_squared = kx*kx;
        w_dot = kx * wind.x - 0.5 * wind.z;

        for(float kz = 0.0; kz < 16.1; kz += 0.5) {
            if (kx != 0.0 || kz != 0.0) {
                w_dot += 0.5 * wind.z;
                k_squared += kz*kz;
                float k = sqrt(k_squared);
                float c = 0.1 * sqrt(exp(-1/k_squared)) * abs(w_dot) / k_squared;
                float w = 2 * sqrt(k);
                float phi2 = phi1 + 7 * rand(vec2(kx, kz)) - w * u_time;
                float dy = c * sin(phi2 + kz * p.z);
                y += dy;
                k_squared -= kz*kz;
                //slope += vec2(kx, kz) * 0.1 * c * sin(phi);
            }
        }
	}
    height = y;
    p += vec4(x,y,z,0);

    gl_Position = u_projection * u_view * u_model * p;
}