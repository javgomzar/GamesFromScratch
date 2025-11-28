cbuffer Globals {
    float4x4 Projection;
    float4x4 View;
    float2 Resolution;
    float2 Mouse;
    float2 LastMouse;
    float Time;
};

cbuffer Transforms {
    float4x4 Model;
    float4x4 Normal;
};

struct VertexIn {
    float3 Position: POSITION;
    float2 Texture: TEXCOORD;
};

struct VertexOut {
    float4 Position: SV_POSITION;
    float2 Texture: TEXCOORD;
};

VertexOut main(VertexIn vin) {
    VertexOut vout;
    vout.Position = mul(mul(mul(float4(vin.Position, 1.0f), Model), View), Projection);
    vout.Texture = vin.Texture;
    return vout;
}
