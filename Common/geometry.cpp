﻿#include "geometry.h"

vec2 cross(const vec2& v1, const vec2& v2) {
    return vec2{v1.x * v2.y - v2.x * v1.y};
}

vec3 cross(const vec3& v1, const vec3& v2) {
    return vec < 3 >
    {
        v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x
    };
}

double dot(const vec2& v1, const vec2& v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

double dot(const vec3& v1, const vec3& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
