#version 400
precision highp float;

double Distance(vec2 p, vec2 q) {
    double p_x = p.x;
    double p_y = p.y;
    double q_x = q.x;
    double q_y = q.y;
    double d_x = p_x - q_x;
    double d_y = p_y - q_y;
    return sqrt(d_x * d_x + d_y * d_y);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
