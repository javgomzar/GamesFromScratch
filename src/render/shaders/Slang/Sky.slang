import Buffers;

struct VS_IN {
    float3 Position: POSITION;
};

struct VS_OUT {
    float4 Position: SV_POSITION;
    float3 WorldPosition: TEXCOORD;
};

[shader ("vertex")]
VS_OUT VSSky(VS_IN vin) {
    VS_OUT vout;

    float4x4 SkyView = View;
    SkyView[3] = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    vout.Position = mul(mul(float4(vin.Position, 1.0f), SkyView), Projection);
    vout.Position.z = vout.Position.w * 0.999999f;
    vout.WorldPosition = vin.Position;

    return vout;
}

static const float PI = 3.14159265359f;

static const float R_EARTH = 6360.0f;     // in km
static const float R_ATMOS = 6420.0f;     // in km
static const float SCALE_HEIGHT_R = 8.5f; // in km
static const float SCALE_HEIGHT_M = 1.2f; // in km
static const float MAX_DISTANCE = sqrt(R_ATMOS * R_ATMOS - R_EARTH * R_EARTH);
static const float g = 0.76f;
static const float C_M = 1.5f * (1.0f - g * g) / (2.0f + g * g);
static const float C_R = 0.7629f;
static const int N_SAMPLES = 16;
static const int N_SAMPLES_LIGHT = 8;
static const float3 BetaR = float3(5.8e-3f, 13.5e-3f, 33.1e-3f);
static const float BetaM = 21e-3f;

[shader("fragment")]
float4 main(VS_OUT vout) : SV_TARGET {
    float3 V = normalize(vout.WorldPosition);
    float3 Sun = -LightDirection;

    float cosA = abs(V.y);
    float cosS = abs(Sun.y);
    float mu = dot(Sun, V);

    // Distance from the viewer to the intersection of the view ray with the atmosphere limit
    float D = sqrt(R_ATMOS * R_ATMOS - R_EARTH * R_EARTH * (1.0f - cosA * cosA)) - R_EARTH * cosA; // in km

    // Rayleigh phase
    float P_R = C_R * (1.0f + 0.9324f * abs(mu) * mu);
    // Mie phase
    float P_M = C_M * (1.0f + abs(mu) * mu) / pow(1.0f + g * g - 2.0f * g * mu, 1.5f);

    float OpticalDepthR = 0;
    float OpticalDepthM = 0;

    float ds = D / N_SAMPLES;
    float3 P = float3(0.0f, R_EARTH, 0.0f) + 0.5f * ds * V;

    float3 sumR = float3(0.0f, 0.0f, 0.0f);
    float3 sumM = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < N_SAMPLES; i++) {
        float H = length(P) - R_EARTH;
        float hR = exp(-H / SCALE_HEIGHT_R) * ds;
        float hM = exp(-H / SCALE_HEIGHT_M) * ds;

        OpticalDepthR += hR;
        OpticalDepthM += hM;

        float RLight = R_EARTH + H;
        float DLight = sqrt(R_ATMOS * R_ATMOS - RLight * RLight * (1.0f - cosS * cosS)) - RLight * cosS;

        float dsLight = DLight / N_SAMPLES_LIGHT;
        float3 PLight = P + 0.5f * dsLight * Sun;
        float OpticalDepthLightR = 0;
        float OpticalDepthLightM = 0;
        for (int j = 0; j < N_SAMPLES_LIGHT; j++) {
            float HLight = length(PLight) - R_EARTH;

            OpticalDepthLightR += exp(-HLight / SCALE_HEIGHT_R) * dsLight;
            OpticalDepthLightM += exp(-HLight / SCALE_HEIGHT_M) * dsLight;

            PLight += dsLight * Sun;
        }

        float3 Tau = exp(-(BetaR * (OpticalDepthR + OpticalDepthLightR) +
                           BetaM * 1.1f * (OpticalDepthM + OpticalDepthLightM)));

        sumR += hR * Tau;
        sumM += hM * Tau;

        P = P + ds * V;
    }

    float3 Color = 1.59155f * (P_R * sumR * BetaR + P_M * sumM * BetaM);

    return float4(Color, 1.0);
}
