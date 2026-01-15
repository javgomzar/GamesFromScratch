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
};

struct GS_OUTPUT {
    float4 Position: SV_POSITION;
    float2 UV: TEXCOORD0;
};

[maxvertexcount(6)]
void main(point VS_OUTPUT Input[1], inout TriangleStream<GS_OUTPUT> OutputStream) {
    float2 Pixel = 1.0f / Resolution;
    float Size = 100.0;
    GS_OUTPUT gout;
    gout.Position = Input[0].Position + Size*float4(-Pixel.x, -Pixel.y, 0.0f, 0.0f); gout.UV = float2(-1.0, -1.0);
    OutputStream.Append(gout);
    gout.Position = Input[0].Position + Size*float4(Pixel.x, -Pixel.y, 0.0f, 0.0f); gout.UV = float2(1.0, -1.0);
    OutputStream.Append(gout);
    gout.Position = Input[0].Position + Size*float4(-Pixel.x, Pixel.y, 0.0f, 0.0f); gout.UV = float2(-1.0, 1.0);
    OutputStream.Append(gout);
    gout.Position = Input[0].Position + Size*float4(Pixel.x, -Pixel.y, 0.0f, 0.0f); gout.UV = float2(1.0, -1.0);
    OutputStream.Append(gout);
    gout.Position = Input[0].Position + Size*float4(-Pixel.x, Pixel.y, 0.0f, 0.0f); gout.UV = float2(-1.0, 1.0);
    OutputStream.Append(gout);
    gout.Position = Input[0].Position + Size*float4(Pixel.x, Pixel.y, 0.0f, 0.0f); gout.UV = float2(1.0, 1.0);
    OutputStream.Append(gout);

    OutputStream.RestartStrip();
}