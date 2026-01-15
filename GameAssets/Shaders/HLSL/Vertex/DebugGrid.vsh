cbuffer Globals: register(b0) {
    float4x4 Projection;
    float4x4 View;
    float2 Resolution;
    float2 Mouse;
    float2 LastMouse;
    float Time;
};

struct VS_IN {
    float3 Position: POSITION;
};

struct VS_OUT {
    float4 Position: SV_POSITION;
    float3 WorldPosition: TEXCOORD;
};

VS_OUT main(VS_IN vin) {
    VS_OUT vout;

    float4x4 SkyView = View;
    SkyView[3] = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    vout.Position = mul(mul(float4(vin.Position, 0.0f), SkyView), Projection);
    vout.Position.z = vout.Position.w * 0.9999f;
    vout.WorldPosition = vin.Position;

    return vout;
}
