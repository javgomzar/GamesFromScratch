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

struct VS_IN {
    float3 Position: POSITION;
    float2 Texture: TEXCOORD;
    float3 Normal: NORMAL;
};

struct VS_OUT {
    float4 Position: SV_POSITION;
    float3 WorldPosition: TEXCOORD1;
    float2 Texture: TEXCOORD0;
    float3 Normal: NORMAL;
};

VS_OUT main(VS_IN vin) {
    VS_OUT vout;
    
    vout.Position = mul(mul(mul(float4(vin.Position, 1.0f), Model), View), Projection);
    vout.WorldPosition = vin.Position;
    vout.Texture = vin.Texture;
    vout.Normal = normalize(mul(float4(vin.Normal, 0.0f), Normal));

    return vout;
}
