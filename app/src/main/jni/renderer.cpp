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


//TODO: add uniforms for proper multiplication for projection * view * model * local_coords
static const char VERTEX_SHADER[] =
        "#version 320 es\n"
        "layout(location = 0) in vec3 a_Position;\n"
        "layout(location = 1) in vec4 a_Color;\n"
        "layout (location = 2) uniform mat4 u_MVP;\n"
        "out vec4 vColor;\n"
        "void main() {\n"
        "    vec4 hom_Pos = vec4(a_Position, 1.0);\n"
        "    gl_Position = u_MVP * hom_Pos;\n"
        "    vColor = a_Color;\n"
        "}\n";

static const char FRAG_SHADER[] =
        "#version 320 es\n"
        "precision mediump float;\n"
        "in vec4 vColor;\n"
        "out vec4 outColor;\n"
        "void main() {\n"
        "    outColor = vColor;\n"
        "}\n";


static GLfloat vertices[] = {
        // vertex          colors
        //  x     y     z     R     G     B     A
       -1.0F,-1.0F, 1.0F, 0.0F, 0.0F, 1.0F, 0.1F, // 5
        1.0F,-1.0F, 1.0F, 1.0F, 0.0F, 1.0F, 0.1F, // 6
        1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.1F, // 7
       -1.0F, 1.0F, 1.0F, 0.0F, 1.0F, 1.0F, 0.1F, // 8
       -1.0F,-1.0F,-1.0F, 0.0F, 0.0F, 0.0F, 0.2F, // 1
        1.0F,-1.0F,-1.0F, 1.0F, 0.0F, 0.0F, 0.2F, // 2
        1.0F, 1.0F,-1.0F, 1.0F, 1.0F, 0.0F, 0.2F, // 3
       -1.0F, 1.0F,-1.0F, 0.0F, 1.0F, 0.0F, 0.2F, // 4
};

GLuint indices[] = {
        // front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3
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
    mShader->Compile();
    mShader->BindShader();

    // fill buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // set attributes
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_DATA_SIZE, (void*)POS_OFFSET);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_DATA_SIZE, (void*)COL_OFFSET);
    glEnableVertexAttribArray(1);

    mShader->UnbindShader();

    ALOGV("Using OpenGL ES 3.0 renderer");
    LOG_INFO("--------- finished init() -----------");
    return true;
}

void Renderer::resize(int w, int h) {
    glViewport(0, 0, w, h);
    LOG_INFO("--------- finished resize() -----------");
}

//TODO: find out correct order of settings-draw-clear-etc
void Renderer::render() {
    glClearColor(0.2F, 0.2F, 0.3F, 1.0F);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glCullFace(GL_FRONT_AND_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw();

    checkGlError("Renderer::render");
    LOG_INFO("--------- render() -----------");
}

void Renderer::draw() {
    mShader->BindShader();
    glBindVertexArray(VAO);

    //TODO: change model, view and projection matrix to uniforms passed into the shader
    //TODO: add slow rotation with some (time function % 360.0F)
    //TODO: add touch listener to change direction of rotation
    //LearnOpenGL
    // matrix definitions

    glm::vec3 axisX = glm::vec3(1.0, 0.0, 0.0);
    glm::vec3 axisY = glm::vec3(0.0, 1.0, 0.0);
    glm::vec3 axisZ = glm::vec3(0.0, 0.0, 1.0);

    glm::mat4 model = glm::mat4(1.0F);
    glm::mat4 view = glm::mat4(1.0F);
    glm::mat4 projection = glm::mat4(1.0F);

    //model = glm::translate(model, glm::vec3(0.0, -0.5, 0.0));
    model = glm::rotate_slow(model, glm::radians(-20.0F), axisY);
    model = glm::rotate_slow(model, glm::radians(15.0F), axisX);
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

    view = glm::translate(view, glm::vec3(0.0, 0.0, -5.0));

    //projection = glm::ortho(-5.0, 5.0, -5.0, 5.0, 0.1, 10.0);
    projection = glm::perspective(glm::radians(45.0f), 1.0f * 9/18, 0.1f, 10.0f);

    //FIXME: this should happen in the shader
    glm::mat4 mvp = projection * view * model;
    // end definitions

    //TODO: find out what "lookAt" does
    //glm::mat4 view = glm::lookAt(glm::vec3(4,3,3), glm::vec3(0,0,0), glm::vec3(0,1,0));

    mShader->PushPositions(POS_OFFSET, VERTEX_DATA_SIZE);
    mShader->PushColors(COL_OFFSET, VERTEX_DATA_SIZE);
    mShader->PushMVPMatrix(&mvp);

    //TODO: draw edges of cube with a *black* line, only edges
    //TODO: --> declare seperate VBO with black as color, maybe use bufferSubData?
    //TODO: replace magic numbers, sizeof() somehow invalidates the draw call
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
    glDrawElements(GL_LINE_STRIP, 36, GL_UNSIGNED_INT, (void*)0);
}

//TODO: delete?
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

//TODO: evaluate use of egl instead of plain gl
/*
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
*/

//TODO: use when maybe switching to egl?
/*
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
}*/
