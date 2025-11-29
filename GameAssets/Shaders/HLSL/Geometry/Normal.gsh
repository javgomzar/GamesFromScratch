struct VS_OUTPUT {
    float4 Position: SV_POSITION;
    float2 Texture: TEXCOORD;
    float3 Normal: NORMAL;
};

struct GS_OUTPUT {
    float4 Position: SV_POSITION;
};

[maxvertexcount(2)]
void main(point VertexOut Input[1], inout PointStream<GeometryOut> OutputStream) {
    GeometryOut gout;
    gout.Position = Input[0].Position;
    OutputStream.Append(gout);
    gout.Position = Input[0].Position + float4(Input[0].Normal, 0);
    OutputStream.Append(gout);
}