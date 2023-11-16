#include "../ShaderLibrary/IShader.hpp"
#include "../ShaderLibrary/ShaderCore.hpp"

class PhongShader : public IShader {
public:
    struct Properties {
        Texture mainTex;
        double gloss;
    } properties;

private:
    struct Atrributes {
        vec4 vertex;
        vec4 normal;
        vec4 texcoord;
    } attributes;

    struct Varyings {
        vec4 posCS; // (x, y) screenUV
        vec4 posWS;
        vec4 nDirWS;
        vec4 uv;
    } varyings;

public:
    PhongShader() {
        bindData(&attributes, &varyings);
        attri_semantic = (int)SEMANTIC::vertex | (int)SEMANTIC::normal | (int)SEMANTIC::texcoord;
    }

    vec4 vertex() override {
        vec4 vertex = attributes.vertex;

        varyings.nDirWS = uniform_ObjcetToWorld * attributes.normal;
        varyings.posCS = uniform_ObjectToClipH * vertex;
        varyings.posWS = uniform_ObjcetToWorld * vertex;
        varyings.uv = attributes.texcoord;

        return varyings.posCS;
    }

    vec4 fragment() override {
        vec3 nDir = varyings.nDirWS.normalized();
        vec3 lDir = (uniform_lightDirection).normalized();
        vec3 vDir = (uniform_worldSpaceCameraPos.xyz() - varyings.posWS.xyz()).normalized();
        vec3 hDir = (lDir + vDir).normalized();

        vec3 albedo = tex2D(properties.mainTex, varyings.uv).xyz();

        vec3 ambient = uniform_ambient.xyz() * albedo;
        vec3 diffuse = uniform_lightColor.xyz() * albedo * saturate(dot(nDir, lDir));
        vec3 specular = uniform_lightColor.xyz() * pow(saturate(dot(nDir, hDir)), properties.gloss);

        vec4 finalColor = ambient + diffuse + specular;

        return finalColor.xyz().Appended<1>({1.0});
    }
};
