struct VS_IN {
    float2 Position: POSITION;
};

struct VS_OUT {
    float3 Position: POSITION;
};

VS_OUT main(VS_IN vin) {
    VS_OUT vout;
	vout.Position = float3(vin.Position.x, 0.0f, vin.Position.y);
    return vout;
}
