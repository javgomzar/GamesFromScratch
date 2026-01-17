cbuffer Globals: register(b0) {
    float4x4 Projection;
    float4x4 View;
    float2 Resolution;
    float2 Mouse;
    float2 LastMouse;
    float Time;
};

struct VS_IN {
    float2 Position: POSITION0;
    float2 Offset: POSITION1;
};

struct VS_OUT {
    float4 Position: SV_POSITION;
    float2 UV: TEXCOORD0;
};

VS_OUT main(VS_IN vin) {
    VS_OUT vout;
    float2 ScreenPosition = vin.Offset + ceil(Resolution / 10.0 * vin.Position);
    float2 Result = (2.0f * float2(ScreenPosition.x, -ScreenPosition.y) / Resolution) + float2(-1.0f, 1.0f);
    vout.Position = float4(Result, 0.0, 1.0);
    vout.UV = vin.Position;
    return vout;
}
