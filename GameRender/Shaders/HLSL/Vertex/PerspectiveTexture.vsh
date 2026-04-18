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
};

struct VS_OUT {
    float4 Position: SV_POSITION;
    float2 Texture: TEXCOORD;
};

VS_OUT main(VS_IN vin) {
    VS_OUT vout;
    vout.Position = mul(mul(mul(float4(vin.Position, 1.0f), Model), View), Projection);
    vout.Texture = vin.Texture;
    return vout;
}
