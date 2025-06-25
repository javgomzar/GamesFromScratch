#version 450

layout (location = 0) in float height;
layout (location = 0) out vec4 frag_color;

void main() {
	float h = 5.0 * height;

	vec4 Blue = vec4(0.0667, 0.0667, 0.4902, 1.0);
	vec4 Green = vec4(0.0863, 0.4549, 0.0863, 1.0);
	vec4 LightBrown =  vec4(0.5176, 0.451, 0.2196, 1.0);
	vec4 Brown = vec4(0.3059, 0.2235, 0.0667, 1.0);
	vec4 White = vec4(1,1,1,1);

	if (h < 0.0001) {
		frag_color = Blue;
	}
	else if (h > 0.0001 && h < 0.1) {
		frag_color = mix(Green, LightBrown, 10*(h - 0.0001));
	}
	else if (h > 0.1 && h < 0.2) {
		frag_color = mix(LightBrown, Brown, 10*(h - 0.1));
	}
	else if (h > 0.2 && h < 0.5) {
		frag_color = mix(Brown, White, (h - 0.2) / 0.3);
	}
	else {
		frag_color = White;
	}
}