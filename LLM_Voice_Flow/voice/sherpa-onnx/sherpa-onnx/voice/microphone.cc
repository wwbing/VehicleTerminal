// sherpa-onnx/csrc/microphone.cc
//
// Copyright (c)  2022-2023  Xiaomi Corporation


/*
    PortAudio是一个免费、跨平台、开源的音频I/O库，用于简化C/C++音频程序的设计实现
    https://blog.csdn.net/GG_SiMiDa/article/details/77185755

*/

#include "sherpa-onnx/csrc/microphone.h"

#include <cstdio>
#include <cstdlib>

#include "portaudio.h"  // NOLINT

namespace sherpa_onnx {

Microphone::Microphone()
{
    // PortAudio库的全局初始化函数，必须在任何音频操作前调用
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "portaudio error: %s\n", Pa_GetErrorText(err));
        exit(-1);
    }
}

Microphone::~Microphone()
{
    //释放PortAudio占用的所有资源（驱动程序、内存等）
    PaError err = Pa_Terminate();
    if (err != paNoError) {
        fprintf(stderr, "portaudio error: %s\n", Pa_GetErrorText(err));
        exit(-1);
    }
}

}  // namespace sherpa_onnx
