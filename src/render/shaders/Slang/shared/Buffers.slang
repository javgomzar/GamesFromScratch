// Constant buffers

cbuffer Globals: register(b0) {
    float4x4 Projection;
    float4x4 View;
    float2 Resolution;
    float2 Mouse;
    float2 LastMouse;
    float Time;
};

cbuffer Light : register(b1) {
    float3 LightDirection;
    float3 LightColor;
    float3 CameraPosition;
    float Ambient;
    float Diffuse;
};

cbuffer Color : register(b2) {
    float4 Color;
}

cbuffer Transforms : register(b3) {
    float4x4 Model;
    float4x4 Normal;
};

static const int MAX_BONES = 32;
cbuffer Bones : register(b4) {
    float4x4 BoneTransforms[MAX_BONES];
    float4x4 BoneNormalTransforms[MAX_BONES];
    int nBones;
};

cbuffer Outline : register(b5) {
    float Width;
    int Level;
};
