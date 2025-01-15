#version 430
precision highp float;

uniform float u_time;

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

void main() {

	float Time = u_time;
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(cos(u_time), sin(u_time), 0.0, 0.0);
	Time += 1.0;
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(cos(Time), sin(Time), 0.0, 0.0);
	Time += 1.0;
	EmitVertex();

	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	
	gl_Position = gl_in[0].gl_Position + vec4(cos(Time), sin(Time), 0.0, 0.0);
	Time += 1.0;
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4(cos(Time), sin(Time), 0.0, 0.0);
	Time += 1.0;
	EmitVertex();

	EndPrimitive();
}