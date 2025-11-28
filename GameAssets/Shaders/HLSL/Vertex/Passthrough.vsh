struct VertexIn {
    float2 Position: POSITION;
    float2 Texture: TEXCOORD0;
};

struct VertexOut {
    float4 Position: SV_POSITION;
    float2 Texture: TEXCOORD0;
};

VertexOut main(VertexIn vin) {
    VertexOut vout;
	vout.Position = float4(vin.Position, 0.0, 1.0);
    vout.Texture = vin.Texture;
    return vout;
}
