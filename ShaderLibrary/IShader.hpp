#ifndef __ISHADER_H__
#define __ISHADER_H__

#include "../tgaimage.h"
#include "../Common/geometry.h"
#include "../Common/texture.hpp"
#include <vector>

// 顶点着色器输入语义
enum class SEMANTIC {
    vertex = 1 << 0,
    normal = 1 << 1,
    tangent = 1 << 2,
    texcoord = 1 << 3,
    color = 1 << 4
};

class IShader {
private:
    std::vector<vec4*> mAttributes, mVaryings;

protected:
    int attri_semantic;

public:
    mat<4, 4> uniform_ObjectToClipH;
    mat<4, 4> uniform_ObjcetToWorld;
    vec4 uniform_lightColor;
    vec4 uniform_lightDirection;
    vec4 uniform_ambient;
    vec4 uniform_worldSpaceCameraPos;

private:
    template <typename T>
    std::vector<vec4*> struct2vector(T* st) {
        std::vector<vec4*> res;
        void* it = st;
        for (int i = 0; i < sizeof(*st) / sizeof(vec4); i++) {
            vec4* elem = (vec4*)it;
            res.push_back(elem);
            it = (void*)((char*)it + sizeof(vec4));
        }
        return res;
    }

protected:
    // 绑定attibutes和varyings的结构体与指针vector
    template <typename T1, typename T2>
    void bindData(T1* Attributes, T2* Varyings) {
        mAttributes = struct2vector(Attributes);
        mVaryings = struct2vector(Varyings);
    }

public:
    IShader() = default;

    virtual ~IShader() {
        for(auto t : mAttributes) {
            t = nullptr;
        }
        for(auto t : mVaryings) {
            t = nullptr;
        }
    }
    
    virtual vec4 vertex() = 0;

    virtual vec4 fragment() = 0;

    int getAttriSemantic() {
        return (int)attri_semantic;
    }

    std::vector<vec4> getAttributes() {
        std::vector<vec4> res;
        for (auto t : mAttributes) res.push_back(*t);
        return res;
    }

    std::size_t getAttributesLen() {
        return mAttributes.size();
    }

    void setAttributes(int i, vec4 attr) {
        *(mAttributes[i]) = attr;
    }

    std::vector<vec4> getVaryings() {
        std::vector<vec4> res;
        for (auto t : mVaryings) res.push_back(*t);
        return res;
    }

    void setVaryings(int i, vec4 vary) {
        *(mVaryings[i]) = vary;
    }

    std::size_t getVaryingsLen() {
        return mVaryings.size();
    }

};
#endif
