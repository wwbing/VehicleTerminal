// sherpa-onnx/csrc/microphone.h
//
// Copyright (c)  2022-2023  Xiaomi Corporation

#ifndef SHERPA_ONNX_CSRC_MICROPHONE_H_
#define SHERPA_ONNX_CSRC_MICROPHONE_H_

/*
  功能:
      基于 PortAudio 的麦克风设备管理功能，是 Sherpa-Onnx 语音识别框架中负责麦克风初始化和释放的模块，
      封装了 PortAudio 库的基础操作
*/

namespace sherpa_onnx {

class Microphone {
public:
    Microphone();
    ~Microphone();
};

}  // namespace sherpa_onnx

#endif  // SHERPA_ONNX_CSRC_MICROPHONE_H_
