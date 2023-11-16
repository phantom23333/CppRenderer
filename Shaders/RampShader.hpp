#include "../ShaderLibrary/IShader.hpp"
#include "../ShaderLibrary/ShaderCore.hpp"

class RampShader : public IShader {
public:
    struct Properties {
        Texture rampTex;
    } properties;

private:
    struct A2v {
        vec4 vertex;
        vec4 normal;
    } a2v;

    struct V2f {
        vec4 posCS; // (x, y) screenUV
        vec4 nDirWS;
    } v2f;

public:
    RampShader() {
        bindData(&a2v, &v2f);
        attri_semantic = (int)SEMANTIC::vertex | (int)SEMANTIC::normal;
    }

    vec4 vertex() override {
        vec4 vertex = a2v.vertex;

        v2f.posCS = uniform_ObjectToClipH * vertex;
        v2f.nDirWS = uniform_ObjcetToWorld * a2v.normal;
        
        return v2f.posCS;
    }

    vec4 fragment() override {
        vec3 nDir = v2f.nDirWS.normalized();
        vec3 lDir = (uniform_lightDirection).normalized();

        double halfLambert = dot(nDir, lDir) * 0.5 + 0.5;

        vec4 var_ramp = tex2D(properties.rampTex, halfLambert);

        vec3 finalColor = var_ramp.xyz();

        return finalColor.Appended<1>({1.0});
    }
};
