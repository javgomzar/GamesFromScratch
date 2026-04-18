#version 450
precision highp float;

layout (location = 0) in vec3 v_position;
layout (location = 0) out vec4 frag_color;

#ifdef VULKAN
layout (std140, set = 0, binding = 1) uniform LightUniforms 
#else
layout (std140, binding = 1) uniform LightUniforms
#endif
{
	vec3 direction;
	vec3 color;
	vec3 cameraPosition;
	float ambient;
	float diffuse;
} LightUBO;

const float PI = 3.14159265359f;

const float R_EARTH = 6360.0f;     // in km
const float R_ATMOS = 6420.0f;     // in km
const float SCALE_HEIGHT_R = 8.5f; // in km
const float SCALE_HEIGHT_M = 1.2f; // in km
const float MAX_DISTANCE = sqrt(R_ATMOS*R_ATMOS - R_EARTH*R_EARTH);
const float g = 0.76f;
const float C_M = 3.0f * (1.0f - g*g) / (2.0f + g*g) / (8.0f * PI);
const float C_R = 3.0f / 16.0f / PI;
const int N_SAMPLES = 16;
const int N_SAMPLES_LIGHT = 8;
const vec3 BetaR = vec3(3.8e-3f, 13.5e-3f, 33.1e-3f);
const float BetaM = 21e-3f;

void main() {
	vec3 V = normalize(v_position);
    vec3 Sun = -LightUBO.direction;

    float cosA = abs(V.y);
    float cosS = abs(Sun.y);
    float mu = dot(Sun, V);

    // Distance from the viewer to the intersection of the view ray with the atmosphere limit
    float D = sqrt(R_ATMOS*R_ATMOS - R_EARTH*R_EARTH*(1.0f - cosA*cosA)) - R_EARTH * cosA; // in km
    
    // Rayleigh phase
    float P_R = C_R * (1.0f + abs(mu) * mu);
    // Mie phase
    float P_M = C_M * (1.0f + abs(mu) * mu) / pow(1.0f + g*g - 2.0f*g*mu, 1.5f);

    float OpticalDepthR = 0;
    float OpticalDepthM = 0;

    float ds = D / N_SAMPLES;
    vec3 P = vec3(0.0f, R_EARTH, 0.0f) + 0.5f * ds * V;

    vec3 sumR = vec3(0.0f, 0.0f, 0.0f);
    vec3 sumM = vec3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < N_SAMPLES; i++) {
        float H = length(P) - R_EARTH;
        float hR = exp(-H / SCALE_HEIGHT_R) * ds;
        float hM = exp(-H / SCALE_HEIGHT_M) * ds;

        OpticalDepthR += hR;
        OpticalDepthM += hM;

        float RLight = R_EARTH + H;
        float DLight = sqrt(R_ATMOS*R_ATMOS - RLight*RLight*(1.0f - cosS*cosS)) - RLight*cosS;

        float dsLight = DLight / N_SAMPLES_LIGHT;
        vec3 PLight = P + 0.5f * dsLight * Sun;
        float OpticalDepthLightR = 0;
        float OpticalDepthLightM = 0;
        for (int j = 0; j < N_SAMPLES_LIGHT; j++) {
            float HLight = length(PLight) - R_EARTH;

            OpticalDepthLightR += exp(-HLight / SCALE_HEIGHT_R) * dsLight;
            OpticalDepthLightM += exp(-HLight / SCALE_HEIGHT_M) * dsLight;

            PLight += dsLight * Sun;
        }

        vec3 Tau = exp(-(BetaR * (OpticalDepthR + OpticalDepthLightR) + 
                     BetaM * 1.1f * (OpticalDepthM + OpticalDepthLightM)));

        sumR += hR * Tau;
        sumM += hM * Tau;

        P = P + ds * V;
    }

    vec3 Color = 20.0f * (P_R * sumR * BetaR + P_M * sumM * BetaM);

    frag_color = vec4(Color, 1.0);
}