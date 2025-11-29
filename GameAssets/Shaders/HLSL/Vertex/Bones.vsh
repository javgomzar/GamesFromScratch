cbuffer Globals: register(b0) {
    float4x4 Projection;
    float4x4 View;
    float2 Resolution;
    float2 Mouse;
    float2 LastMouse;
    float Time;
};

cbuffer Transforms: register(b3) {
    float4x4 Model;
    float4x4 Normal;
};

struct VertexIn {
    float3 Position: POSITION;
    float2 Texture: TEXCOORD;
    float3 Normal: NORMAL;
    int2 BoneIDs: BONEIDS;
    float2 BoneWeights: BONEWEIGHTS;
};

struct VertexOut {
    float4 Position: SV_POSITION;
    float3 WorldPosition: TEXCOORD1;
    float2 Texture: TEXCOORD;
    float3 Normal: NORMAL;
};

VertexOut main(VertexIn vin) {
    VertexOut vout;
    
    vout.Position = mul(mul(mul(float4(vin.Position, 1.0f), Model), View), Projection);
    vout.WorldPosition = vin.Position;
    vout.Texture = vin.Texture;
    vout.Normal = normalize(mul(float4(vin.Normal, 0.0f), Normal));

    return vout;
}
