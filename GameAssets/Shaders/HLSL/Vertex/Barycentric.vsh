cbuffer Globals {
    float4x4 Projection;
    float4x4 View;
    float2 Resolution;
    float2 Mouse;
    float2 LastMouse;
    float Time;
};

cbuffer Text: register(b7) {
    float2 Pen;
	float Size;
};

struct VertexIn {
    float2 Position: POSITION;
    float2 Barycentric: TEXCOORD;
};

struct VertexOut {
    float4 Position: SV_POSITION;
    float2 Barycentric: TEXCOORD;
};

VertexOut main(VertexIn vin) {
    VertexOut vout;
    vout.Barycentric = vin.Barycentric;

    float2 Position = float2(vin.Position.x, -vin.Position.y);
	float2 Sized = Pen + Size * vin.Position;
    float2 Result = (2 * float2(Sized.x, -Sized.y) / Resolution) + float2(-1.0, 1.0);
	vout.Position = float4(Result, 0.0, 1.0);
    return vout;
}
