#define Tau 6.283185307179586476925287

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
    float4 Properties: POSITION1;
};

struct VS_OUT {
    float4 Position: SV_POSITION;
    float2 UV: TEXCOORD0;
    float Hue: COLOR;
};

VS_OUT main(VS_IN vin) {
    VS_OUT vout;
    vout.UV = vin.Position;
    vout.Hue = vin.Properties.z;

    float4x4 SkyView = View;
    SkyView[3] = float4(0.0f, 0.0f, 0.0f, 1.0f);

    float RightAscension = Tau * vin.Properties.x / 24.0;
    float Declination = Tau * vin.Properties.y / 360.0f;
    float Size = vin.Properties.w;
    
    float3 Position = float3(cos(RightAscension) * cos(Declination), sin(Declination), sin(RightAscension) * cos(Declination));
    float4 SkyPosition = mul(mul(float4(Position, 1.0f), SkyView), Projection);
    SkyPosition.z = SkyPosition.w * 0.99f;

    vout.Position = SkyPosition + Size * float4(vin.Position / Resolution, 0.0, 0.0);

    return vout;
}
