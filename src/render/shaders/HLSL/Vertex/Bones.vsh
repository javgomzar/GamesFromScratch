cbuffer Globals: register(b0) {
    float4x4 Projection;
    float4x4 View;
    float2 Resolution;
    float2 Mouse;
    float2 LastMouse;
    float Time;
};

cbuffer Transforms: register(b3) {
    float4x4 Model;
    float4x4 Normal;
};

static const int MAX_BONES = 32;
cbuffer Bones: register(b4) {
	float4x4 BoneTransforms[MAX_BONES];
	float4x4 BoneNormalTransforms[MAX_BONES];
	int nBones;
};

struct VS_IN {
    float3 Position: POSITION;
    float2 Texture: TEXCOORD;
    float3 Normal: NORMAL;
    int2 BoneIDs: BONEIDS;
    float2 BoneWeights: BONEWEIGHTS;
};

struct VS_OUT {
    float4 Position: SV_POSITION;
    float3 WorldPosition: TEXCOORD1;
    float2 Texture: TEXCOORD;
    float3 Normal: NORMAL;
};

VS_OUT main(VS_IN vin) {
    VS_OUT vout;
    vout.WorldPosition = vin.Position;
    vout.Texture = vin.Texture;
    vout.Normal = normalize(mul(float4(vin.Normal, 0.0f), Normal));

    float3 Position = vin.Position;
    if (nBones > 0) {
        // Positions
        float4 First = mul(float4(vin.Position, 1.0f), BoneTransforms[vin.BoneIDs[0]]);
        float4 Second = mul(float4(vin.Position, 1.0f), BoneTransforms[vin.BoneIDs[1]]);
        Position = First * vin.BoneWeights[0] + Second * vin.BoneWeights[1];

        // Normals
        float4 FirstNormal = mul(float4(vin.Normal, 0.0f), BoneNormalTransforms[vin.BoneIDs[0]]);
		float4 SecondNormal = mul(float4(vin.Normal, 0.0f), BoneNormalTransforms[vin.BoneIDs[1]]);
        float4 N = vin.BoneWeights[0] * FirstNormal + vin.BoneWeights[1] * SecondNormal;
		vout.Normal = normalize(mul(N, Normal)).xyz;
    }
    
    vout.Position = mul(mul(mul(float4(Position, 1.0f), Model), View), Projection);

    return vout;
}
