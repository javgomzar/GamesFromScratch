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
	vout.Position = float4(vin.Position, 0.0, 1.0);
    vout.Texture = vin.Texture;
    return vout;
}
