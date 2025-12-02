struct VS_IN {
    float3 Position: POSITION;
    float2 Texture: TEXCOORD0;
};

struct VS_OUT {
    float3 Position: POSITION;
    float2 Texture: TEXCOORD0;
};

VS_OUT main(VS_IN vin) {
    VS_OUT vout;
	vout.Position = vin.Position;
    vout.Texture = vin.Texture;
    return vout;
}
