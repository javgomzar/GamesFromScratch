#version 450

#ifdef VULKAN
layout(std140, set = 2, binding = 2) uniform TextUniforms 
#else 
layout(std140, binding = 8) uniform TextUniforms 
#endif
{
    vec2 Pen;
	float Size;
} TextUBO;

layout(vertices = 3) out;

void main() {
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = 1;
        
        vec4 Middle = 0.5 * (gl_in[0].gl_Position + gl_in[2].gl_Position);
        float D = length(Middle - gl_in[1].gl_Position);
        
        int Level = 4;
        if (D < 0.0001) {
            Level = 1;
        }
        else if (TextUBO.Size > 0.1) {
            Level = 8;
        }
        gl_TessLevelOuter[1] = Level;
    }
}
