import Buffers;


struct VS_IN {
    float3 Position : POSITION;
    float2 Texture : TEXCOORD;
    float3 Normal : NORMAL;
};

struct VS_BONES_IN {
    float3 Position : POSITION;
    float2 Texture : TEXCOORD;
    float3 Normal : NORMAL;
    int2 BoneIDs : BONEIDS;
    float2 BoneWeights : BONEWEIGHTS;
};

struct VS_OUT {
    float4 Position : SV_POSITION;
    float3 WorldPosition : TEXCOORD1;
    float2 Texture : TEXCOORD;
    float3 Normal : NORMAL;
};

[shader("vertex")]
VS_OUT VSMesh(VS_IN vin) {
    VS_OUT vout;

    vout.Position = mul(mul(mul(float4(vin.Position, 1.0f), Model), View), Projection);
    vout.WorldPosition = vin.Position;
    vout.Texture = vin.Texture;
    vout.Normal = normalize(mul(float4(vin.Normal, 0.0f), Normal)).xyz;

    return vout;
}

[shader("vertex")]
VS_OUT VSBones(VS_BONES_IN vin) {
    VS_OUT vout;
    vout.WorldPosition = vin.Position;
    vout.Texture = vin.Texture;
    vout.Normal = normalize(mul(float4(vin.Normal, 0.0f), Normal)).xyz;

    float3 Position = vin.Position;
    if (nBones > 0) {
        // Positions
        float4 First = mul(float4(vin.Position, 1.0f), BoneTransforms[vin.BoneIDs[0]]);
        float4 Second = mul(float4(vin.Position, 1.0f), BoneTransforms[vin.BoneIDs[1]]);
        Position = (First * vin.BoneWeights[0] + Second * vin.BoneWeights[1]).xyz;

        // Normals
        float4 FirstNormal = mul(float4(vin.Normal, 0.0f), BoneNormalTransforms[vin.BoneIDs[0]]);
        float4 SecondNormal = mul(float4(vin.Normal, 0.0f), BoneNormalTransforms[vin.BoneIDs[1]]);
        float4 N = vin.BoneWeights[0] * FirstNormal + vin.BoneWeights[1] * SecondNormal;
        vout.Normal = normalize(mul(N, Normal)).xyz;
    }

    vout.Position = mul(mul(mul(float4(Position, 1.0f), Model), View), Projection);

    return vout;
}
