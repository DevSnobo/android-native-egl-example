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
#include <jni.h>
#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer

#include "jniapi.h"
#include "logger.h"
#include "renderer.h"

//static ANativeWindow *window = nullptr;
static Renderer *g_renderer = nullptr;

static void printGlString(const char* name, GLenum s) {
    const char* v = (const char*)glGetString(s);
    LOG_INFO("GL %s: %s\n", name, v);
}

extern "C"
JNIEXPORT void JNICALL
Java_tsaarni_nativeeglexample_DemoLIB_init(JNIEnv *env, jclass type) {
    if (g_renderer) {
        delete g_renderer;
        g_renderer = NULL;
    }

    printGlString("Version", GL_VERSION);
    LOG_INFO("%s", "------------\n");
    printGlString("Vendor", GL_VENDOR);
    LOG_INFO("%s", "------------\n");
    printGlString("Renderer", GL_RENDERER);
    LOG_INFO("%s", "------------\n");
    printGlString("Extensions", GL_EXTENSIONS);
    LOG_INFO("%s", "------------\n");

    const char* versionStr = (const char*)glGetString(GL_VERSION);
    if (strstr(versionStr, "OpenGL ES 3.")) {
        g_renderer = createRenderer();
    } else {
        LOG_ERROR("Unsupported OpenGL ES version");
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_tsaarni_nativeeglexample_DemoLIB_resize(JNIEnv *env, jclass type, jint width, jint height) {
    if (g_renderer) {
        g_renderer->resize(width, height);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_tsaarni_nativeeglexample_DemoLIB_step(JNIEnv *env, jclass type) {
    if (g_renderer) {
        g_renderer->render();
    }
}

