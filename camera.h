#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "Common/geometry.h"

// 视锥体信息 假设z为正方向
struct _frustum {
    double aspect; // 长宽比
    double fov; // 垂直角度 弧度制
};

// 相机信息
struct _camera {
    vec3 worldPos; // 世界坐标 右手坐标系
    vec3 pos; // 左手坐标系
    vec4 rect; // (x, y, w, h)
    double near, far;
    _frustum frustum;

    vec3 L, R, U; // 观察向量 右向量 上向量

    mat<4, 4> view;
    mat<4, 4> projection;
    mat<4, 4> viewport;

    bool dirtyFlag; // 更新标记

    _camera() = default;

    _camera(vec3 worldPos, double aspect, double fov, double near, double far, vec4 rect) : worldPos(worldPos), near(near), far(far), rect(rect) {
        frustum.aspect = aspect;
        frustum.fov = fov;
        pos = {worldPos.x, worldPos.y, -worldPos.z};// inverse z
        dirtyFlag = true;// 更新标记
    }

    void lookAt(vec3 target) {
        L = (target - pos).normalized();
        R = cross({0, 1, 0}, L).normalized(); // 右向量
        U = cross(L, R).normalized(); // 上向量

        dirtyFlag = true;
    }

    // 计算视图矩阵
    void updateView() {
        L = L.normalized(); // 观察方向
        U = cross(L, R).normalized(); // 上向量
        R = cross(L, U).normalized(); // 右向量

        auto& v = view;
        v = mat<4, 4>::identity();
        v[0][0] = R.x;
        v[0][1] = R.y;
        v[0][2] = R.z;
        v[0][3] = -pos.x * R.x - pos.y * R.y - pos.z * R.z;
        v[1][0] = U.x;
        v[1][1] = U.y;
        v[1][2] = U.z;
        v[1][3] = -pos.x * U.x - pos.y * U.y - pos.z * U.z;
        v[2][0] = -L.x;
        v[2][1] = -L.y;
        v[2][2] = -L.z;
        v[2][3] = pos.x * L.x + pos.y * L.y + pos.z * L.z;
    }

    // 计算透视投影矩阵
    void updateProjection() {
        auto& p = projection;
        p = mat<4, 4>::identity();
        p[0][0] = 1.0 / tan(frustum.fov / 2.0) * frustum.aspect;
        p[1][1] = 1.0 / tan(frustum.fov / 2.0);
        p[2][2] = (near + far) / (near - far);
        p[2][3] = (2 * near * far) / (near - far);
        p[3][2] = -1;
        p[3][3] = 0;
    }

    // 计算视口矩阵
    void updateViewport() {
        auto& v = viewport;
        v = mat<4, 4>::identity();
        v[0][0] = rect[2] / 2.0;
        v[0][3] = rect[2] / 2.0;
        v[1][1] = rect[3] / 2.0 + rect[0];
        v[1][3] = rect[3] / 2.0 + rect[1];
    }

    void updated() {
        if (!dirtyFlag) return;
        updateView();
        updateProjection();
        updateViewport();
    }
};

#endif