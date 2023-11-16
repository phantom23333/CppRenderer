#include <iostream>
#include <cmath>
#include <vector>
#include "tgaimage.h"
#include "model.h"
#include "camera.h"
#include "Common/geometry.h"
#include "Common/texture.hpp"
#include "ShaderLibrary/IShader.hpp"
#include "Shaders/PhongShader.hpp"
#include "Shaders/RampShader.hpp"

const TGAColor white = {255, 255, 255, 255};
const TGAColor red = {0, 0, 255, 255};

// 计算重心坐标系系数(1-u-v, u, v) P = (1-u-v)A+uB+vC
vec3 barycentric(vec4* pts, vec3 p) {
    vec3 v1 = {pts[1].x - pts[0].x, pts[2].x - pts[0].x, pts[0].x - p.x}, v2 = {pts[1].y - pts[0].y, pts[2].y - pts[0].y, pts[0].y - p.y};
    vec3 t = cross(v1, v2);
    // 如果叉乘结果的z分量为0，则不能作为归一化分子
    if (std::abs(t.z) < 1e-2) return {-1, 1, 1}; // 返回负坐标，在光栅化中舍弃
    return {1.0 - (t.x + t.y) / t.z, t.x / t.z, t.y / t.z}; // 返回系数
}

double interpolate(const vec3& val, const vec3& weight) {
    double res = 0;
    for (int i = 0; i < 3; i++) res += val[i] * weight[i];
    return res;
}

// 渲染
template <typename T>
void render(Model* model, TGAImage& image, T& shader) {
    // shader必须是IShader的子列
    static_assert(std::is_base_of<IShader, T>::value, "T must be a subclass of IShader");

    // 缓存RT分辨率
    double width = image.width(), height = image.height();

    _camera camera({3, -4, -5}, 1.77, 1.04, 0.01, 1000, {0, 0, width, height});
    camera.lookAt({0, 0, 0});
    camera.updated();

    // 顶点着色器后的屏幕映射坐标集合
    std::vector<vec4> posSS;
    // Shader用于插值的Varyings数据集合
    std::vector<std::vector<vec4>> varyingss;

    // TODO: 应该是渲染管线的工作而不是Render本身
    // 传shader全局属性
    shader.uniform_ObjectToClipH = camera.projection * camera.view;
    shader.uniform_ObjcetToWorld = camera.view;
    shader.uniform_lightColor = {1, 0.8, 0.8, 1};
    shader.uniform_lightDirection = vec4{0, 0, 1, 0}.normalized();
    shader.uniform_ambient = {0.1, 0.1, 0.1, 1.0};
    shader.uniform_worldSpaceCameraPos = {camera.pos.x, camera.pos.y, camera.pos.z, 1};

    // 几何阶段
    // vertex shader
    for (int i = 0; i < model->nfaces(); i++) {
        // 三角形是否被剔除
        bool clipped = true;
        // 三角行的屏幕映射坐标和用于插值的数据
        vec4 tmp_posSS[3];
        std::vector<vec4> tmp_varyingss[3];
        for (int j = 0; j < 3; j++) {
            // 传递着色器attributes
            auto semantic = shader.getAttriSemantic();
            // 要求必须输入顶点语义
            assert(semantic & (int)SEMANTIC::vertex);
            int attr_i = 0; // attributes下标计数
            // vertex
            auto vertex = model->vert(i, j);
            shader.setAttributes(attr_i++, {vertex.x, vertex.y, vertex.z, 1});
            // normal
            auto normal = model->normal(i, j);
            if (semantic & (int)SEMANTIC::normal) {
                shader.setAttributes(attr_i++, {normal.x, normal.y, normal.z, 0});
            }
            // TODO: model类没有加读tangent

            // texcoord
            auto texcoord = model->uv(i, j);
            if (semantic & (int)SEMANTIC::texcoord) {
                shader.setAttributes(attr_i++, {texcoord.x, texcoord.y, 0, 0});
            }

            // TODO: model类没有加顶点色

            // 调用顶点着色器
            auto posCS = shader.vertex();

            double& w = posCS[3];
            // 判断这个顶点是否会被透视投影剔除
            if (posCS[0] >= -w && posCS[0] <= w && posCS[1] >= -w && posCS[1] <= w && posCS[2] >= -w && posCS[2] <= w)
                clipped = false;
            // 进行齐次除法
            if (w < 1e-7) w = 1e-7;
            for (int i = 0; i < 3; i++) posCS[i] /= w;
            w = 1;

            // 屏幕映射
            tmp_posSS[j] = discretize(camera.viewport * posCS);
            // 存储顶点着色器处理后的用于插值的值
            tmp_varyingss[j] = shader.getVaryings();
        }
        // 如果整个三角形都没被剔除 则存储数据
        if (!clipped) {
            for (int j = 0; j < 3; j++) {
                posSS.push_back(tmp_posSS[j]);
                varyingss.push_back(tmp_varyingss[j]);
            }
        }
    }

    // 光栅化阶段
    double* zbuffer = new double[width * height];
    for (int i = 0; i < width * height; i++) zbuffer[i] = 1;
    // 三角形遍历
    for (int tri = 0; tri < posSS.size(); tri += 3) {
        // 三角形选择
        vec4 pts[3] = {posSS[tri], posSS[tri + 1], posSS[tri + 2]};
        std::vector<vec4> varyings[3] = {varyingss[tri], varyingss[tri + 1], varyingss[tri + 2]};

        // 计算AABB
        vec2 bboxmin = {image.width() - 1.0, image.height() - 1.0};
        vec2 bboxmax = {-1, -1};
        vec2 clamp = {image.width() - 1.0, image.height() - 1.0};
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 2; j++) {
                bboxmin[j] = std::max(0.0, std::min(bboxmin[j], pts[i][j]));
                bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
            }
        }

        // 遍历像素
        vec3 p;
        for (p.x = bboxmin.x; p.x <= bboxmax.x; p.x++) {
            for (p.y = bboxmin.y; p.y <= bboxmax.y; p.y++) {
                // 计算重心坐标系数
                vec3 weight = barycentric(pts, p);
                // 舍弃不在三角形内的像素
                if (weight.x < 0 || weight.y < 0 || weight.z < 0) continue;

                // z test
                p.z = interpolate({pts[0].z, pts[1].z, pts[2].z}, weight);
                // 由于相机右手坐标系 -z为forward 所以zbuffer为负 越大的越前面
                if (zbuffer[int(p.x + p.y * width)] < p.z) continue;
                // z write
                zbuffer[int(p.x + p.y * width)] = p.z;

                // 重心坐标插值 将Shader端的Varyings全部进行插值
                for (int i = 0; i < shader.getVaryingsLen(); i++) {
                    vec4 varying;
                    for (int j = 0; j < 4; j++)
                        varying[j] = interpolate({varyings[0][i][j], varyings[1][i][j], varyings[2][i][j]}, weight);

                    shader.setVaryings(i, varying);
                }

                // fragment shader
                vec4 fragment = saturate(shader.fragment());

                // 计算最终颜色
                TGAColor fragment_color = {uint8_t(fragment[2] * 255), uint8_t(fragment[1] * 255), uint8_t(fragment[0] * 255), 255};

                image.set(p.x, p.y, fragment_color);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    const int width = 800;
    const int height = 800;

    TGAImage phongImg(width, height, TGAImage::RGB);
    TGAImage rampImg(width, height, TGAImage::RGB);

    Model* model = new Model("obj/african_head/african_head.obj");

    // 加载贴图
    Texture diffuse("obj/african_head/african_head_diffuse.tga");
    Texture ramp("obj/african_head/Green Ramp Texture.tga");

    PhongShader phongShader;
    phongShader.properties.mainTex = diffuse;
    phongShader.properties.gloss = 300;

    RampShader rampShader;
    rampShader.properties.rampTex = ramp;

    render(model, rampImg, rampShader);
    render(model, phongImg, phongShader);

    phongImg.write_tga_file("framebuffer_phong.tga");
    rampImg.write_tga_file("framebuffer_ramp.tga");

    return 0;
}
