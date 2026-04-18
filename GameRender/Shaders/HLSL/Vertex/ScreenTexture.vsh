cbuffer Globals: register(b0) {
    float4x4 Projection;
    float4x4 View;
    float2 Resolution;
    float2 Mouse;
    float2 LastMouse;
    float Time;
};

struct VS_IN {
    float2 Position: POSITION;
    float2 Texture: TEXCOORD0;
};

struct VS_OUT {
    float4 Position: SV_POSITION;
    float2 Texture: TEXCOORD0;
};

VS_OUT main(VS_IN vin) {
    VS_OUT vout;
    vout.Position = float4((2.0f * float2(vin.Position.x, -vin.Position.y) / Resolution) + float2(-1.0f, 1.0f), 0.0, 1.0);
    vout.Texture = float2(vin.Texture.x, 1.0 - vin.Texture.y);
	return vout;
}
