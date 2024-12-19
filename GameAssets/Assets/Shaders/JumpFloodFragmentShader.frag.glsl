
layout(binding = 0) uniform sampler2D binded_texture;
layout(binding = 1) uniform sampler2D attachment_texture;

const int maxSteps = 12;

uniform vec2 u_resolution;
uniform int level;

out vec4 frag_color;

in vec2 v_position;
in vec2 v_texture;

vec3 StepJFA(vec2 p) {
    vec3 closest = vec3(-1.0, 0.0, 0.0);
    double closest_distance = 9999999.0;
    float w = level;
    for (int y = -1; y <= 1; ++y) {
    for (int x = -1; x <= 1; ++x) {
        vec2 t = p + w * vec2(x,y);
        if (t.x >= 0. && t.y >= 0. && t.x <= u_resolution.x && t.y <= u_resolution.y) {
            vec4 stored_color = texture(binded_texture, t / u_resolution);
            if (stored_color.b > 0.) { // skip computing for yet invalid seeds
                vec2 stored_position = stored_color.rg;
                double d = distance(p, stored_position);
                if (closest_distance > d) {
                    closest_distance = d;
                    closest = stored_color.rgb;
                }
            }
        }
    }}

    return closest;
}

void main() {
    vec3 closest = StepJFA(gl_FragCoord.xy);
    if (closest.x < 0.) {
        frag_color = texture(binded_texture, v_texture);
    }
    else {
        frag_color = vec4(closest, 1.0);
    }
}
