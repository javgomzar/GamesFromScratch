import Buffers;


struct VS_IN {
    float2 Position : POSITION;
};

struct VS_TEX_IN {
    float2 Position : POSITION;
    float2 Texture : TEXCOORD;
};

struct VS_OUT {
    float4 Position : SV_POSITION;
    float2 Texture : TEXCOORD;
}

[shader("vertex")]
VS_OUT VSPassthrough(VS_IN vin) {
    VS_OUT vout;
    vout.Position = float4(vin.Position, 0.0f, 1.0f);
    return vout;
}

[shader("vertex")]
VS_OUT VSPassthroughTexture(VS_TEX_IN vin) {
    VS_OUT vout;
    vout.Position = float4(vin.Position, 0.0f, 1.0f);
    vout.Texture = vin.Texture;
    return vout;
}

[shader("vertex")]
VS_OUT VSScreen(VS_IN vin) {
    VS_OUT vout;
    vout.Position = float4((2.0f * float2(vin.Position.x, -vin.Position.y) / Resolution) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return vout;
}

[shader("vertex")]
VS_OUT VSScreenTexture(VS_TEX_IN vin) {
    VS_OUT vout;
    vout.Position = float4((2.0f * float2(vin.Position.x, -vin.Position.y) / Resolution) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    vout.Texture = float2(vin.Texture.x, 1.0f- vin.Texture.y);
    return vout;
}

Texture2DMS<float4, 8> TextureMS : register(t0);

[shader("pixel")]
float4 PSAntialiasing(VS_OUT vout) : SV_Target {
    int2 Coord = int2(vout.Position.xy);
    float4 Mean = float4(0, 0, 0, 0);
    for (int i = 0; i < 8; i++) {
        float4 Sample = TextureMS.Load(Coord, i);
        Mean += Sample.a * Sample;
    }

    return 0.125 * Mean;
}

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

[shader("pixel")]
float4 main(VS_OUT vout) : SV_TARGET {
    return Texture.Sample(Sampler, float2(vout.Texture.x, 1.0 - vout.Texture.y));
}