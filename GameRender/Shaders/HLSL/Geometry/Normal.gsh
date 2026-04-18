cbuffer Globals: register(b0) {
    float4x4 Projection;
    float4x4 View;
    float2 Resolution;
    float2 Mouse;
    float2 LastMouse;
    float Time;
};

struct VS_OUTPUT {
    float4 Position: SV_POSITION;
    float3 WorldPosition: TEXCOORD1;
    float2 Texture: TEXCOORD0;
    float3 Normal: NORMAL;
};

struct GS_OUTPUT {
    float4 Position: SV_POSITION;
};

[maxvertexcount(2)]
void main(point VS_OUTPUT Input[1], inout LineStream<GS_OUTPUT> OutputStream) {
    GS_OUTPUT gout;
    gout.Position = Input[0].Position;
    OutputStream.Append(gout);
    gout.Position = Input[0].Position + mul(mul(float4(Input[0].Normal, 0), View), Projection);
    OutputStream.Append(gout);

    OutputStream.RestartStrip();
}