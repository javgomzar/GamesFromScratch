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

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

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
	vec2 st = 20. * gl_FragCoord.xy/GlobalUBO.resolution;

	vec2 d = vec2(0.);
	d.x = fbm(.1*st + vec2(1.8, .1));
	d.y = fbm(.1*st + vec2(1.1, 2.8));

	vec2 r = vec2(0.);
	r.x = fbm(1. * st + 1.1 * d + vec2(2.1, 1.8) + 0.05 * GlobalUBO.time);
	r.y = fbm(1. * st + 1.2 * d + vec2(2., 1.) + 0.05 * GlobalUBO.time);

	float intensity = fbm(st + r + 0.2 * GlobalUBO.time) + 0.1;
	intensity *= 1.6 * intensity;

	// vec3 color = mix(
	// 	vec3(2. * intensity, 0.0, 0.0),
	// 	vec3(2. * intensity, intensity, 0.),
	// 	intensity
	// );

	vec3 color = hsv2rgb(vec3( .1 * intensity, 1, 1));

	frag_color = vec4(color, 1.);
}