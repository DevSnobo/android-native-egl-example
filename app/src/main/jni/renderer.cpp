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

#include <cstdint>
#include <unistd.h>
#include <pthread.h>
#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "logger.h"
#include "renderer.h"

constexpr GLuint VERTEX_DATA_SIZE = (7 * sizeof(float));
constexpr int POS_OFFSET = (0 * sizeof(float));
constexpr int COL_OFFSET = (3 * sizeof(float));


//TODO: write shader for rendering floating cube
//FIXME: current error: "cannot convert from float to vec4"
static const char VERTEX_SHADER[] =
        "#version 300 es\n"
        "layout(location = 0) in vec3 a_Position;\n"
        "layout(location = 1) in vec4 a_Color;\n"
        "uniform mat4 u_MVP;\n"
        "out vec4 vColor;\n"
        "void main() {\n"
        "float x = a_Position.x;\n"
        "float y = a_Position.y;\n"
        "float z = a_Position.z;\n"
        "    vec4 tmp = vec4(x, y, z, x);\n" //removed mvp matrix to test
        "    gl_Position = vec3(tmp.xyz);\n"
        "    vColor = a_Color;\n"
        "}\n";

//TODO: write shader for rendering floating cube
static const char FRAG_SHADER[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 vColor;\n"
        "out vec4 outColor;\n"
        "void main() {\n"
        "    outColor = vColor;\n"
        "}\n";


static GLfloat vertices[] = {
        // vertex          colors
        //  x     y     z     R     G     B     A
        -1.0F,-1.0F,-1.0F, 0.0F, 0.0F, 0.0F, 1.0F,
         1.0F,-1.0F,-1.0F, 1.0F, 0.0F, 0.0F, 1.0F,
         1.0F, 1.0F,-1.0F, 1.0F, 1.0F, 0.0F, 1.0F,
        -1.0F, 1.0F,-1.0F, 0.0F, 1.0F, 0.0F, 1.0F,
        -1.0F,-1.0F, 1.0F, 0.0F, 0.0F, 1.0F, 1.0F,
         1.0F,-1.0F, 1.0F, 1.0F, 0.0F, 1.0F, 1.0F,
         1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F,
        -1.0F, 1.0F, 1.0F, 0.0F, 1.0F, 1.0F, 1.0F
};

GLushort indices[] = {
        0, 4, 5, 0, 5, 1,
        1, 5, 6, 1, 6, 2,
        2, 6, 7, 2, 7, 3,
        3, 7, 4, 3, 4, 0,
        4, 7, 6, 4, 6, 5,
        3, 0, 1, 3, 1, 2
};

GLuint VAO; GLuint VBO; GLuint EBO;

bool checkGlError(const char *funcName) {
    GLint err = glGetError();
    if (err != GL_NO_ERROR) {
        ALOGE("GL error after %s(): 0x%08x\n", funcName, err);
        return true;
    }
    return false;
}

Renderer *createRenderer() {
    auto *renderer = new Renderer;
    if (!renderer->init()) {
        delete renderer;
        return nullptr;
    }
    return renderer;
}

Renderer::Renderer() {
}

Renderer::~Renderer() = default;

bool Renderer::init() {
    mShader = new Shader(VERTEX_SHADER, FRAG_SHADER);
    mCube = new SimpleGeom(new VertexBuf(vertices, sizeof(vertices), VERTEX_DATA_SIZE),
                           new IndexBuf(indices, sizeof(indices)));
    mCube->vbuf->SetColorsOffset(COL_OFFSET);
    mShader->Compile();
    mShader->BindShader();

    //TODO: fill buffers

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_DATA_SIZE, (void*)POS_OFFSET);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_DATA_SIZE, (void*)COL_OFFSET);
    glEnableVertexAttribArray(1);

    mShader->UnbindShader();

    ALOGV("Using OpenGL ES 3.0 renderer");
    return true;
}

void Renderer::resize(int w, int h) {
    glViewport(0, 0, w, h);
}

void Renderer::render() {
    //FIXME: is this useful?
    //step();
    glClearColor(0.2F, 0.2F, 0.3F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw();
    checkGlError("Renderer::render");
}

void Renderer::draw() {
    mShader->BindShader();
    //mCube->vbuf->SetPrimitive(GL_TRIANGLE_STRIP);
    //mShader->BeginRender(mCube->vbuf);

    /* glm::vec4 vec = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
     glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 0.0f));
     vec = trans * vec;
     glm::mat4 viewMat = glm::lookAt(glm::vec3(0,0,10), glm::vec3(0,0,9), glm::vec3(-sin(0.1f), 0, cos(-0.f)));*/

    /*glm::mat4 id = glm::mat4(1.0F);
    glm::mat4 trans = glm::translate(id, glm::vec3(0, 0, -10.0));
    mShader->Render(mCube->ibuf, &trans);*/

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
}

/*void Renderer::calcSceneParams(unsigned int w, unsigned int h,
                               float *offsets) {
    // number of cells along the larger screen dimension
    const float NCELLS_MAJOR = MAX_INSTANCES_PER_SIDE;
    // cell size in scene space
    const float CELL_SIZE = 2.0F / NCELLS_MAJOR;

    // Calculations are done in "landscape", i.e. assuming dim[0] >= dim[1].
    // Only at the end are values put in the opposite order if h > w.
    const float dim[2] = {fmaxf(w, h), fminf(w, h)};
    const float aspect[2] = {dim[0] / dim[1], dim[1] / dim[0]};
    const float scene2clip[2] = {1.0F, aspect[0]};
    const int ncells[2] = {
            static_cast<int>(NCELLS_MAJOR),
            (int) floorf(NCELLS_MAJOR * aspect[1])
    };

    float centers[2][MAX_INSTANCES_PER_SIDE];
    for (int d = 0; d < 2; d++) {
        auto offset = -ncells[d] / NCELLS_MAJOR; // -1.0 for d=0
        for (auto i = 0; i < ncells[d]; i++) {
            centers[d][i] = scene2clip[d] * (CELL_SIZE * (i + 0.5F) + offset);
        }
    }

    int major = w >= h ? 0 : 1;
    int minor = w >= h ? 1 : 0;
    // outer product of centers[0] and centers[1]
    for (int i = 0; i < ncells[0]; i++) {
        for (int j = 0; j < ncells[1]; j++) {
            int idx = i * ncells[1] + j;
            offsets[2 * idx + major] = centers[0][i];
            offsets[2 * idx + minor] = centers[1][j];
        }
    }

    mNumInstances = ncells[0] * ncells[1];
    mScale[major] = 0.5F * CELL_SIZE * scene2clip[0];
    mScale[minor] = 0.5F * CELL_SIZE * scene2clip[1];
}*/

/*void Renderer::step() {
    timespec now{};
    clock_gettime(CLOCK_MONOTONIC, &now);
    auto nowNs = now.tv_sec * 1000000000ULL + now.tv_nsec;

    if (mLastFrameNs > 0) {
        float dt = float(nowNs - mLastFrameNs) * 0.000000001f;

        for (unsigned int i = 0; i < mNumInstances; i++) {
            mAngles[i] += mAngularVelocity[i] * dt;
            if (mAngles[i] >= TWO_PI) {
                mAngles[i] -= TWO_PI;
            } else if (mAngles[i] <= -TWO_PI) {
                mAngles[i] += TWO_PI;
            }
        }

        float *transforms = mapTransformBuf();
        for (unsigned int i = 0; i < mNumInstances; i++) {
            float s = sinf(mAngles[i]);
            float c = cosf(mAngles[i]);
            transforms[4 * i + 0] = c * mScale[0];
            transforms[4 * i + 1] = s * mScale[1];
            transforms[4 * i + 2] = -s * mScale[0];
            transforms[4 * i + 3] = c * mScale[1];
        }
        unmapTransformBuf();
    }

    mLastFrameNs = nowNs;
}*/





/*

void Renderer::renderLoop() {
    bool renderingEnabled = true;

    LOG_INFO("renderLoop()");
    while (renderingEnabled) {

        pthread_mutex_lock(&_mutex);

        // FIXME: thread locks are weird
        // process incoming messages
        switch (_msg) {

            case MSG_WINDOW_SET:
                LOG_INFO("CASE_WINDOW_SET");
                initialize();
                break;

            case MSG_RENDER_LOOP_EXIT:
                LOG_INFO("CASE_EXIT");
                renderingEnabled = false;
                destroy();
                break;

            default:
                LOG_INFO("CASE_DEFAULT");
                break;
        }
        _msg = MSG_NONE;

        if (_display) {
            drawFrame();
                LOG_ERROR("NOW drawing frame ...");
            if (!eglSwapBuffers(_display, _surface)) {
                LOG_ERROR("eglSwapBuffers() returned error %d", eglGetError());
            }
        } else {
            LOG_ERROR("----- NOT DRAWING -----");
        }

        pthread_mutex_unlock(&_mutex);
        //eglSwapBuffers(_display, _surface);
        LOG_ERROR("----- NOT DRAWING -----");
    }

    LOG_INFO("Render loop exits");
}

bool Renderer::initialize() {
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLDisplay display;
    EGLConfig config;
    EGLint numConfigs;
    EGLint format;
    EGLSurface surface;
    EGLContext context;
    EGLint width;
    EGLint height;
    GLfloat ratio;

    LOG_INFO("Initializing context");

    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        LOG_ERROR("eglGetDisplay() returned error %d", eglGetError());
        return false;
    }
    if (!eglInitialize(display, nullptr, nullptr)) {
        LOG_ERROR("eglInitialize() returned error %d", eglGetError());
        return false;
    }

    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
        LOG_ERROR("eglChooseConfig() returned error %d", eglGetError());
        destroy();
        return false;
    }

    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
        LOG_ERROR("eglGetConfigAttrib() returned error %d", eglGetError());
        destroy();
        return false;
    }

    ANativeWindow_setBuffersGeometry(_window, 0, 0, format);

    if (!(surface = eglCreateWindowSurface(display, config, _window, nullptr))) {
        LOG_ERROR("eglCreateWindowSurface() returned error %d", eglGetError());
        destroy();
        return false;
    }

    if (!(context = eglCreateContext(display, config, nullptr, nullptr))) {
        LOG_ERROR("eglCreateContext() returned error %d", eglGetError());
        destroy();
        return false;
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        LOG_ERROR("eglMakeCurrent() returned error %d", eglGetError());
        destroy();
        return false;
    }

    if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
        !eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
        LOG_ERROR("eglQuerySurface() returned error %d", eglGetError());
        destroy();
        return false;
    }

    _display = display;
    _surface = surface;
    _context = context;

    // FIXME: need to reimplement rendering with modern OpenGL
    glDisable(GL_DITHER);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glClearColor(0, 0, 0, 0);
    glEnable(GL_CULL_FACE);
    //glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, width, height);

    // FIXME: Shader declaration
    _shader = new Shader("shaders/shader.vs", "shaders/shader.fs");
    _shader.Compile();

    _cube = new SimpleGeom(new VertexBuf(vertices, sizeof(vertices), 7 * sizeof(GLfloat)),
                           new IndexBuf(indices, sizeof(indices)));
    _cube->vbuf->SetColorsOffset(3 * sizeof(GLfloat));


    */
/*glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid *) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat),
                          (GLvoid *) (3 * sizeof(GLfloat)));*//*


    ratio = (GLfloat) width / height;

    */
/* glm::vec4 vec = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
     glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 0.0f));
     vec = trans * vec;
     glm::mat4 viewMat = glm::lookAt(glm::vec3(0,0,10), glm::vec3(0,0,9), glm::vec3(-sin(0.1f), 0, cos(-0.f)));*//*


    */
/*glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustumf(-ratio, ratio, -1, 1, 1, 10);*//*


    return true;
}

void Renderer::destroy() {
    LOG_INFO("Destroying context");

    eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(_display, _context);
    eglDestroySurface(_display, _surface);
    eglTerminate(_display);

    _display = EGL_NO_DISPLAY;
    _surface = EGL_NO_SURFACE;
    _context = EGL_NO_CONTEXT;
}

void Renderer::drawFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // FIXME: need to reimplement rendering with modern OpenGL
    _shader.BindShader();
    _shader.BeginRender(_cube->vbuf);
    glm::mat4 id = glm::mat4(1.0f);
    glm::mat4 res = glm::translate(id, glm::vec3(0, 0, -10));
    _shader.Render(_cube->ibuf, &res);


    */
/*glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -3.0f);
    glRotatef(_angle, 0, 1, 0);
    glRotatef(_angle * 0.25f, 1, 0, 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glFrontFace(GL_CW);
    glVertexPointer(3, GL_FIXED, 0, vertices);
    glColorPointer(4, GL_FIXED, 0, colors);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, indices);
    *//*


    _shader.EndRender();
    _angle += 1.2f;
}

void *Renderer::threadStartCallback(void *myself) {
    auto *renderer = (Renderer *) myself;
    LOG_INFO("Callback function");

    renderer->renderLoop();
    pthread_exit(nullptr);
}

*/
