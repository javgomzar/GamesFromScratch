#version 430
precision highp float;

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) readonly uniform image2D source;
layout(rgba32f, binding = 0) writeonly uniform image2D target;

uniform float u_time;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec4 value = imageLoad(source, texelCoord);
	
    float x = float(texelCoord.x)/(gl_NumWorkGroups.x);
    float y = float(texelCoord.y)/(gl_NumWorkGroups.y);
	
    if (value.a > 0.) {
        value = vec4(x,y,0.,1.);
    }
    imageStore(target, texelCoord, value);
}