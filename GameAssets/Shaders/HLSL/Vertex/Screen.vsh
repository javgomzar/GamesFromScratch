cbuffer Globals {
    float4x4 Projection;
    float4x4 View;
    float2 Resolution;
    float2 Mouse;
    float2 LastMouse;
    float Time;
};

struct VertexIn {
    float2 Position: POSITION;
};

float4 main(VertexIn vin) : SV_POSITION {
    float2 Result = (2.0f * float2(vin.Position.x, -vin.Position.y) / Resolution) + float2(-1.0f, 1.0f);
	return float4(Result, 0, 1.0);
}
