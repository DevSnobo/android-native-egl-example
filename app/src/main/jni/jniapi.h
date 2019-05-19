//
// Copyright 2019 Robin Fritz
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
#ifndef JNIAPI_H
#define JNIAPI_H

extern "C" {
JNIEXPORT void JNICALL
Java_tsaarni_nativeeglexample_DemoLIB_init(JNIEnv *env, jclass type, jint width, jint height);

JNIEXPORT void JNICALL
Java_tsaarni_nativeeglexample_DemoLIB_resize(JNIEnv *env, jclass type, jint width, jint height);

JNIEXPORT void JNICALL
Java_tsaarni_nativeeglexample_DemoLIB_render(JNIEnv *env, jclass type);

JNIEXPORT void JNICALL
Java_tsaarni_nativeeglexample_DemoLIB_pushAngle(JNIEnv *env, jclass clazz, jfloat angle);
};
#endif // JNIAPI_H
