#version 450
precision highp float;

#define OCTAVES 8

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

layout (location = 0) out vec4 frag_color;

float random (in vec2 st) {
    return fract(sin(dot(st.xy,vec2(12.9898,78.233)))*43758.5453123);
}

float noise(in vec2 st) {
	vec2 i = floor(st);
	vec2 f = fract(st);

	float a = random(i);
	float b = random(i + vec2(1.,0.));
	float c = random(i + vec2(0.,1.));
	float d = random(i + vec2(1.,1.));

	vec2 u = f * f * (3. - 2. * f);

	return mix(a, b, u.x) + (c - a) * u.y * (1. - u.x) + (d - b) * u.x * u.y;
}

float fbm(in vec2 st) {
	float value = 0.;
	float amplitude = .5;
	float frequency = 0.;

	for (int i = 0; i < OCTAVES; i++) {
		value += amplitude * noise(st);
		st *= 2.;
		amplitude *= .5;
	}
	return value;
}

void main() {
	vec2 st = 5. * gl_FragCoord.xy/GlobalUBO.resolution;

	vec3 color = vec3(fbm(st));

	frag_color = vec4(color,1.);
}