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
#include <android/native_window.h> // requires ndk r5 or newer
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "logger.h"
#include "renderer.h"

#define LOG_TAG "EglSample"

static GLfloat vertices[] = {
        -0x10000, -0x10000, -0x10000, 0x00000, 0x00000, 0x00000, 0x10000,
        0x10000, -0x10000, -0x10000, 0x10000, 0x00000, 0x00000, 0x10000,
        0x10000, 0x10000, -0x10000, 0x10000, 0x10000, 0x00000, 0x10000,
        -0x10000, 0x10000, -0x10000, 0x00000, 0x10000, 0x00000, 0x10000,
        -0x10000, -0x10000, 0x10000, 0x00000, 0x00000, 0x10000, 0x10000,
        0x10000, -0x10000, 0x10000, 0x10000, 0x00000, 0x10000, 0x10000,
        0x10000, 0x10000, 0x10000, 0x10000, 0x10000, 0x10000, 0x10000,
        -0x10000, 0x10000, 0x10000, 0x00000, 0x10000, 0x10000, 0x10000
};

GLushort indices[] = {
        0, 4, 5, 0, 5, 1,
        1, 5, 6, 1, 6, 2,
        2, 6, 7, 2, 7, 3,
        3, 7, 4, 3, 4, 0,
        4, 7, 6, 4, 6, 5,
        3, 0, 1, 3, 1, 2
};

Renderer::Renderer()
        : _msg(MSG_NONE), _display(nullptr), _surface(nullptr), _context(nullptr), _shader(nullptr),
          _cube(nullptr), _angle(0) {
    LOG_INFO("Renderer instance created");
}

Renderer::~Renderer() {
    LOG_INFO("Renderer instance destroyed");
}

void Renderer::start() {
    LOG_INFO("Creating renderer thread");
    pthread_create(&_threadId, nullptr, threadStartCallback, this);
}

void Renderer::stop() {
    LOG_INFO("Stopping renderer thread");

    // send message to render thread to stop rendering
    pthread_mutex_lock(&_mutex);
    _msg = MSG_RENDER_LOOP_EXIT;
    pthread_mutex_unlock(&_mutex);

    pthread_join(_threadId, nullptr);
    LOG_INFO("Renderer thread stopped");
}

void Renderer::setWindow(ANativeWindow *window) {
    // notify render thread that window has changed
    pthread_mutex_lock(&_mutex);
    _msg = MSG_WINDOW_SET;
    _window = window;
    pthread_mutex_unlock(&_mutex);
}


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
        }

        pthread_mutex_unlock(&_mutex);
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
                          (GLvoid *) (3 * sizeof(GLfloat)));*/

    ratio = (GLfloat) width / height;

    /* glm::vec4 vec = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
     glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 0.0f));
     vec = trans * vec;
     glm::mat4 viewMat = glm::lookAt(glm::vec3(0,0,10), glm::vec3(0,0,9), glm::vec3(-sin(0.1f), 0, cos(-0.f)));*/

    /*glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustumf(-ratio, ratio, -1, 1, 1, 10);*/

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
    */

    _shader.EndRender();
    _angle += 1.2f;
}

void *Renderer::threadStartCallback(void *myself) {
    auto *renderer = (Renderer *) myself;
    LOG_INFO("Callback function");

    renderer->renderLoop();
    pthread_exit(nullptr);
}

