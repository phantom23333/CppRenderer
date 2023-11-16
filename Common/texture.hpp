#ifndef __TEXTURE__H__
#define __TEXTURE__H__

#include <string>
#include "tgaimage.h"
#include "geometry.h"

class Texture {
private:
    TGAImage mTextrure;
    std::string mFilepath;
    double mWidth, mHeight;
    vec2 mResolution;
public:
    Texture() = default;

    Texture(std::string filepath) {
        loadTexture(filepath);
    }

    ~Texture() = default;

    bool loadTexture(std::string filepath) {
        if (!mTextrure.read_tga_file(filepath.c_str()))
            return false;
        mFilepath = filepath;
        mWidth = (double)mTextrure.width();
        mHeight = (double)mTextrure.height();
        mResolution = {mWidth, mHeight};
        return true;
    }

    std::string filepath() const {
        return mFilepath;
    }

    double width() const {
        return mWidth;
    }

    double height() const {
        return mHeight;
    }

    vec2 resolution() const {
        return mResolution;
    }

    TGAColor get(const int& x, const int& y) const {
        return mTextrure.get(x, y);
    }
};


#endif
