#version 450

layout (vertices = 4) out;

layout(location = 1) in vec2 v_texture[];
layout(location = 0) out vec2 texture_coord[4];

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    texture_coord[gl_InvocationID] = v_texture[gl_InvocationID];

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = 64;
        gl_TessLevelOuter[1] = 64;
        gl_TessLevelOuter[2] = 64;
        gl_TessLevelOuter[3] = 64;
        gl_TessLevelInner[0] = 64;
        gl_TessLevelInner[1] = 64;
    }
}

