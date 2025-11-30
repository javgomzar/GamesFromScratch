struct VertexIn {
    float3 Position: POSITION;
    float2 Texture: TEXCOORD0;
};

struct VertexOut {
    float3 Position: POSITION;
    float2 Texture: TEXCOORD0;
};

VertexOut main(VertexIn vin) {
    VertexOut vout;
	vout.Position = vin.Position;
    vout.Texture = vin.Texture;
    return vout;
}
