#ifndef __SHADER_CORE__
#define __SHADER_CORE__

#include "../Common/geometry.h"
#include "../Common/texture.hpp"

vec4 tex2D(const Texture& map, vec2 uv) {
    vec4 res;
    TGAColor var_map = map.get(uv.x * map.width(), uv.y * map.height());
    for (int i = 0; i < 3; i++) res[i] = var_map[2 - i] / 255.0;
    res[3] = var_map[3] / 255.0;
    return res;
}

double clamp(double x, double a, double b) {
    return x < a ? a : (x > b ? b : x);
}

double saturate(double x) {
    return clamp(x, 0, 1);
}

template<int n> vec<n> saturate(const vec<n> &x) {
    vec<n> res = x;
    for(int i = 0; i < n; i++) res[i] = saturate(res[i]);
    return res;
}

#endif