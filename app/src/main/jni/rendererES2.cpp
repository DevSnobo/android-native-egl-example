//
// Created by snobo on 5/6/19.
//

#include "renderer.h"
#include "rendering/common.hpp"

class RendererES2: public Renderer {
public:
    RendererES2();
    virtual ~RendererES2();
    bool init();

private:
    virtual float* mapOffsetBuf();
    virtual void unmapOffsetBuf();
    virtual float* mapTransformBuf();
    virtual void unmapTransformBuf();
    virtual void draw(unsigned int numInstances);

    const EGLContext mEglContext;
    GLuint mProgram;
    GLuint mVB;
    GLint mPosAttrib;
    GLint mColorAttrib;
    GLint mScaleRotUniform;
    GLint mOffsetUniform;

    float mOffsets[2*MAX_INSTANCES];
    float mScaleRot[4*MAX_INSTANCES];   // array of 2x2 column-major matrices
};