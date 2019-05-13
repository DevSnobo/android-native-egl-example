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
#include <chrono>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "logger.h"
#include "renderer.h"

static const char VERTEX_SHADER[] =
        "#version 320 es\n"
        "layout(location = 0) in vec3 a_Position;\n"
        "layout(location = 1) in vec4 a_Color;\n"
        "layout (location = 2) uniform mat4 u_Model;\n"
        "layout (location = 3) uniform mat4 u_View;\n"
        "layout (location = 4) uniform mat4 u_Projection;\n"
        "out vec4 vColor;\n"
        "void main() {\n"
        "    vec4 hom_Pos = vec4(a_Position, 1.0);\n"
        "    gl_Position = u_Projection * u_View * u_Model * hom_Pos;\n"
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

GLfloat vertices[] = {
        // vertex
        //  x     y     z
        -1.0F, -1.0F,  1.0F,
         1.0F, -1.0F,  1.0F,
         1.0F,  1.0F,  1.0F,
        -1.0F,  1.0F,  1.0F,
        -1.0F, -1.0F, -1.0F,
         1.0F, -1.0F, -1.0F,
         1.0F,  1.0F, -1.0F,
        -1.0F,  1.0F, -1.0F,
};

GLfloat colors_cube[] = {
        0.0F, 0.0F, 1.0F, 0.1F,
        1.0F, 0.0F, 1.0F, 0.1F,
        1.0F, 1.0F, 1.0F, 0.1F,
        0.0F, 1.0F, 1.0F, 0.1F,
        0.0F, 0.0F, 0.0F, 0.2F,
        1.0F, 0.0F, 0.0F, 0.2F,
        1.0F, 1.0F, 0.0F, 0.2F,
        0.0F, 1.0F, 0.0F, 0.2F,
};

GLfloat colors_edges[] = {
        0.0F, 0.0F, 0.0F, 0.5F,
        0.0F, 0.0F, 0.0F, 0.5F,
        0.0F, 0.0F, 0.0F, 0.5F,
        0.0F, 0.0F, 0.0F, 0.5F,
        0.0F, 0.0F, 0.0F, 0.5F,
        0.0F, 0.0F, 0.0F, 0.5F,
        0.0F, 0.0F, 0.0F, 0.5F,
        0.0F, 0.0F, 0.0F, 0.5F,
};

GLuint indices_cube[] = {
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

GLuint indices_edges[] = {
        0, 1, // front
        1, 2,
        2, 3,
        3, 0,
        4, 5, // back
        5, 6,
        6, 7,
        7, 4,
        0, 4, // sides
        1, 5,
        2, 6,
        3, 7,

};

std::chrono::system_clock::time_point start;
GLuint VAO_cube;
GLuint VAO_edges;
GLuint VBO_cube;
GLuint VBO_edges;
GLuint EBO_cube;
GLuint EBO_edges;

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

    GLuint VAOs[2];
    GLuint VBOs[2];
    GLuint EBOs[2];

    //---------------------
    // generate objects
    glGenVertexArrays(2, VAOs);
    VAO_cube = VAOs[0];
    VAO_edges = VAOs[1];

    glGenBuffers(2, VBOs);
    VBO_cube = VBOs[0];
    VBO_edges = VBOs[1];

    glGenBuffers(2, EBOs);
    EBO_cube = EBOs[0];
    EBO_edges = EBOs[1];

    //---------------------
    //bind first VAO for cube
    glBindVertexArray(VAO_cube);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_cube);

    //fill buffer with separate arrays
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors_cube), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors_cube), &colors_cube);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_cube);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_cube), indices_cube, GL_STATIC_DRAW);

    // set attributes
    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    // color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) sizeof(vertices));
    glBindVertexArray(0);

    //---------------------
    //bind second VAO for edges
    glBindVertexArray(VAO_edges);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_edges);

    //fill buffer with separate arrays
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors_edges), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors_edges), &colors_edges);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_edges);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_edges), indices_edges, GL_STATIC_DRAW);

    // set attributes
    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    // color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) sizeof(vertices));
    glBindVertexArray(0);

    //---------------------

    mShader->UnbindShader();

    start = std::chrono::system_clock::now();

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

    std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> end = std::chrono::system_clock::now();
    long long int degrees = std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start).count();

    //TODO: add touch listener to change direction of rotation

    // LearnOpenGL
    // matrix definitions
    glm::vec3 axisX = glm::vec3(1.0, 0.0, 0.0);
    glm::vec3 axisY = glm::vec3(0.0, 1.0, 0.0);
//    glm::vec3 axisZ = glm::vec3(0.0, 0.0, 1.0);

    glm::mat4 model = glm::mat4(1.0F);
    glm::mat4 view = glm::mat4(1.0F);
    glm::mat4 projection = glm::mat4(1.0F);

    model = glm::rotate_slow(model, glm::radians(15.0F), axisX);
    model = glm::rotate_slow(model, glm::radians(-(degrees * 1.0f) / 40), axisY);
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

    view = glm::translate(view, glm::vec3(0.0, 0.0, -5.0));

    projection = glm::perspective(glm::radians(45.0f), 1.0f * 9 / 18, 0.1f, 10.0f);
    // end definitions

    mShader->PushPositions(0, (3 * sizeof(float)));
    mShader->PushColors(sizeof(vertices), 4 * sizeof(float));

    mShader->PushModelMatrix(&model);
    mShader->PushViewMatrix(&view);
    mShader->PushProjectionMatrix(&projection);

    glBindVertexArray(VAO_cube);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void *) 0);
    glBindVertexArray(0);
    glBindVertexArray(VAO_edges);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, (void *) 0);
    glBindVertexArray(0);
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

    glDisable(GL_DITHER);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glClearColor(0, 0, 0, 0);
    glEnable(GL_CULL_FACE);
    //glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, width, height);
}*/
