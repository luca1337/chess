#ifndef VEC_H
#define VEC_H

typedef struct vec2 {
    float xy[2];
} vec2_t;

vec2_t vec2_create(float, float);
vec2_t vec2_add(vec2_t, vec2_t);
vec2_t vec2_sub(vec2_t, vec2_t);
vec2_t vec2_scale(vec2_t, vec2_t);
vec2_t vec2_scaled(vec2_t, float);
float vec2_dot(vec2_t, vec2_t);
vec2_t vec2_reflected(vec2_t, vec2_t);
float vec2_slow_len(vec2_t);
float vec2_fast_len(vec2_t);
vec2_t vec2_normalize(vec2_t);
void vec2_interpolate(vec2_t* v0, vec2_t* v1, float t);

#endif