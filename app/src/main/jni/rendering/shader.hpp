/*
 * Copyright (C) Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef demo_shader_hpp
#define demo_shader_hpp

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "../glm/glm.hpp"
#include "simplegeom.hpp"

class VertexBuf;

class IndexBuf;

/* Represents an OpenGL shader. This class is not meant to be used directly, but, rather
 * to be subclassed to represent specific shaders. To use any shader that's a subclass
 * of this class, first construct it, then call Compile(). After that, you can render
 * geometry by calling BeginRender(), then calling Render() as many times as you want,
 * and then EndRender(). This allows you to render the same geometry in multiple
 * places efficiently. If you just want to render a geometry once (simple use case),
 * you can call RenderSimpleGeom(). */
class Shader {
protected:
    // OpenGL handles
    GLuint mVertShaderH, mFragShaderH;
    const char *mVertSource;
    const char *mFragSource;
    GLuint mProgramH;

    int mModelMatrixLoc;
    int mViewMatrixLoc;
    int mProjectionMatrixLoc;
    GLint mPositionLoc;
    GLint mColorLoc;
    GLint mTexCoordLoc;
    GLint mScaleRot;
    GLint mOffset;
    int mTintLoc;
    int mSamplerLoc;
    int mPointLightPosLoc;
    int mPointLightColorLoc;

    // Geometry we are rendering (this is only valid between BeginRender and EndRender)
    VertexBuf *mPreparedVertexBuf;
public:
    Shader();
    Shader(const char *vertexSource, const char *fragSource);
    virtual ~Shader();

    // compile shader
    virtual void Compile();

    // rendering:
    void BindShader();
    void UnbindShader();

    // Push methods for uniforms and attributes
    // Push Model, View and Projection matrices to the shader
    void PushModelMatrix(glm::mat4 *mat);
    void PushViewMatrix(glm::mat4 *mat);
    void PushProjectionMatrix(glm::mat4 *mat);

    // Push the vertex positions to the shader
    void PushPositions(int vbo_offset, int stride);

    // Push the vertex colors to the shader
    void PushColors(int vbo_offset, int stride);

    // Push the vertex textures to the shader
    void PushTextures(int vbo_offset, int stride);

    GLuint GetShaderId();

protected:


    // Must return the vertex shader's GLSL source
    virtual const char *GetVertShaderSource();

    // Must return the fragment shader's GLSL source
    virtual const char *GetFragShaderSource();

    // Must return the shader's name (used for debug/logging purposes)
    virtual const char *GetShaderName();
};

#endif

