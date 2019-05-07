//
// Copyright 2011 Tero Saarni
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <pthread.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "rendering/shader.hpp"
#include "rendering/simplegeom.hpp"

// Include the latest possible header file( GL version header )
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

#define DEBUG 1

#define LOG_TAG "DemoRenderer"
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#if DEBUG
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#else
#define ALOGV(...)
#endif

#define MAX_INSTANCES_PER_SIDE 16
#define MAX_INSTANCES   (MAX_INSTANCES_PER_SIDE * MAX_INSTANCES_PER_SIDE)
#define TWO_PI          (2.0 * M_PI)
#define MAX_ROT_SPEED   (0.3 * TWO_PI)



class Renderer {
public:
    Renderer();
    virtual ~Renderer();
    bool init();
    void resize(int w, int h);
    void render();

protected:
    /*// return a pointer to a buffer of MAX_INSTANCES * sizeof(vec2).
    // the buffer is filled with per-instance offsets, then unmapped.
    virtual float* mapOffsetBuf();
    virtual void unmapOffsetBuf();
    // return a pointer to a buffer of MAX_INSTANCES * sizeof(vec4).
    // the buffer is filled with per-instance scale and rotation transforms.
    virtual float* mapTransformBuf();

    virtual void unmapTransformBuf();

    virtual void draw(unsigned int numInstances);*/
private:
    Shader *mShader;
    SimpleGeom *mCube;

    void draw();

    /*unsigned int mNumInstances;
    float mScale[2]{};
    float mAngularVelocity[MAX_INSTANCES]{};
    uint64_t mLastFrameNs{};
    float mAngles[MAX_INSTANCES]{};*/
};

extern bool checkGlError(const char* funcName);
extern Renderer* createRenderer();

/*class Renderer {

public:
    Renderer();

    virtual ~Renderer();

    // Following methods can be called from any thread.
    // They send message to render thread which executes required actions.
    void start();

    void stop();

    void setWindow(ANativeWindow *window);
    void resize(int w, int h);
    void render();


private:

    enum RenderThreadMessage {
        MSG_NONE = 0,
        MSG_WINDOW_SET,
        MSG_RENDER_LOOP_EXIT
    };

    pthread_t _threadId{};
    pthread_mutex_t _mutex;
    enum RenderThreadMessage _msg;

    // android window, supported by NDK r5 and newer
    ANativeWindow *_window{};

    EGLDisplay _display;
    EGLSurface _surface;
    EGLContext _context;
    SimpleGeom *_cube;
    GLfloat _angle;

    // RenderLoop is called in a rendering thread started in start() method
    // It creates rendering context and renders scene until stop() is called
    void renderLoop();

    bool initialize();

    void destroy();

    void drawFrame();

    // Helper method for starting the thread 
    static void *threadStartCallback(void *myself);

};*/


#endif // RENDERER_H
