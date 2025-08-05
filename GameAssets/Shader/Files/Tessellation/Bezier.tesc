#version 450

#ifdef VULKAN
layout(std140, set = 2, binding = 2) uniform TextUniforms 
#else 
layout(std140, binding = 8) uniform TextUniforms 
#endif
{
	float DPI;
    float Points;
} TextUBO;

layout (vertices = 3) out;

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = 1;
        int Level = 4;
        if (TextUBO.Points > 80) {
            Level = 8;
        }
        gl_TessLevelOuter[1] = Level;
    }
}
