#include "vec2.h"
#include <math.h>

vec2_t vec2_create(float x, float y)
{
    vec2_t result;
    result.xy[0] = x;
    result.xy[1] = y;
    return result;
}

vec2_t vec2_add(vec2_t a, vec2_t b)
{
    return vec2_create(a.xy[0] + b.xy[0], a.xy[1] + b.xy[1]);
}

vec2_t vec2_sub(vec2_t a, vec2_t b)
{
    return vec2_create(a.xy[0] - b.xy[0], a.xy[1] - b.xy[1]);
}

vec2_t vec2_scale(vec2_t a, vec2_t b)
{
    return vec2_create(a.xy[0] * b.xy[0], a.xy[1] * b.xy[1]);
}

vec2_t vec2_scaled(vec2_t v, float s)
{
    return vec2_create(v.xy[0] * s, v.xy[1] * s);
}

float vec2_dot(vec2_t a, vec2_t b)
{
    return a.xy[0] * b.xy[0] + a.xy[1] + b.xy[1];
}

vec2_t vec2_reflected(vec2_t v, vec2_t n)
{
    return vec2_sub(v, vec2_scaled(n, 2 * vec2_dot(v, n)));
}

float vec2_slow_len(vec2_t v)
{
    return (float)sqrt(pow(v.xy[0], 2) + pow(v.xy[1], 2));
}

float vec2_fast_len(vec2_t v)
{
    return (pow(v.xy[0], 2) + pow(v.xy[1], 2));
}

vec2_t vec2_normalize(vec2_t v)
{
    float v_len = vec2_slow_len(v);
    v.xy[0] = v.xy[0] / v_len;
    v.xy[1] = v.xy[1] / v_len;
    return v;
}

void vec2_interpolate(vec2_t* v0, vec2_t* v1, float t)
{
    v0->xy[0] += v1->xy[0] * t;
    v0->xy[1] += v1->xy[1] * t;
}