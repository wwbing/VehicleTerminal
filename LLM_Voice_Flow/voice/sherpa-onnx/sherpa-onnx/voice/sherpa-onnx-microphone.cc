/*
    1. 语音采集​：通过 PortAudio 捕获麦克风音频流

    2. 实时识别​：使用 Sherpa-Onnx 流式模型进行语音识别，生成文本.
        Sherpa-Onnx是ZipFormer的推理框架，选的是ZipFormer模型
            Zipformer在LibriSpeech、Aishell-1和WenetSpeech等常用数据集上取得了当前最好的ASR结果。
            选取的是流式、混合中英支持、small尺度、int8量化的zipformer模型

    3. 结果处理​：识别结果通过 ZeroMQ 发送到 LLM/TTS 服务

    5. 控制流​：支持中断信号处理和动态等待机制
*/

#include <csignal>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <clocale>
#include <cwctype>

#include "ZmqClient.h"
#include "portaudio.h"  // NOLINT
#include "sherpa-onnx/csrc/display.h"
#include "sherpa-onnx/csrc/microphone.h"
#include "sherpa-onnx/csrc/online-recognizer.h"

static bool stop             = false;
static float mic_sample_rate = 16000;
static bool wait             = false;

// potraudio 回调函数：异步调用（20ms周期），该回调把 硬件音频 直接送入 zipformer的音频流
static auto RecordCallback(const void *input_buffer, void *, unsigned long frames_per_buffer,
                           const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *user_data) -> int32_t
{
    // 实现音频采集和TTS播放的互斥
    if (!wait) {
        // PortAudio 回调要求 void* 通用指针，因此需要强制转换为语音识别流水线专用的 OnlineStream 对象指针
        auto *stream = reinterpret_cast<sherpa_onnx::OnlineStream *>(user_data);

        // 音频数据注入zipformer
        stream->AcceptWaveform(static_cast<int32_t>(mic_sample_rate), reinterpret_cast<const float *>(input_buffer),
                               frames_per_buffer);
    }

    return stop ? paComplete : paContinue;
}

// 信号处理函数：处理Ctrl+C等信号，优雅退出程序
static void Handler(int32_t /*sig*/)
{
    stop = true;
    fprintf(stderr, "\nCaught Ctrl + C. Exiting...\n");
}

// Unicode字符串转小写函数：支持多语言字符的大小写转换
static auto tolowerUnicode(const std::string &input_str) -> std::string
{
    // Use system locale
    std::setlocale(LC_ALL, "");

    // From char string to wchar string
    std::wstring input_wstr(input_str.size() + 1, '\0');
    std::mbstowcs(&input_wstr[0], input_str.c_str(), input_str.size());
    std::wstring lowercase_wstr;

    for (wchar_t wc : input_wstr) {
        if (std::iswupper(wc)) {
            lowercase_wstr += std::towlower(wc);
        } else {
            lowercase_wstr += wc;
        }
    }

    // Back to char string
    std::string lowercase_str(input_str.size() + 1, '\0');
    std::wcstombs(&lowercase_str[0], lowercase_wstr.c_str(), lowercase_wstr.size());

    return lowercase_str;
}

auto main(int32_t argc, char *argv[]) -> int32_t
{
    // 注册信号处理器，处理Ctrl+C退出
    signal(SIGINT, Handler);

    // 初始化ZMQ客户端，用于与LLM和TTS系统通信
    zmq_component::ZmqClient client;                                // 和LLM交互：默认127.0.0.1:6666
    zmq_component::ZmqClient block_client("tcp://localhost:6677");  // 和TTS交互

    const char *kUsageMessage = R"usage(
    This program uses streaming models with microphone for speech recognition.
    Usage:

    ./bin/sherpa-onnx-microphone \
        --tokens=/path/to/tokens.txt \
        --encoder=/path/to/encoder.onnx \
        --decoder=/path/to/decoder.onnx \
        --joiner=/path/to/joiner.onnx \
        --provider=cpu \
        --num-threads=1 \
        --decoding-method=greedy_search

    Please refer to
    https://k2-fsa.github.io/sherpa/onnx/pretrained_models/index.html
    for a list of pre-trained models to download.
    )usage";

    // 解析命令行参数和配置
    sherpa_onnx::ParseOptions po(kUsageMessage);
    sherpa_onnx::OnlineRecognizerConfig config;

    config.Register(&po);
    po.Read(argc, argv);
    if (po.NumArgs() != 0) {
        po.PrintUsage();
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "%s\n", config.ToString().c_str());

    // 验证配置参数
    if (!config.Validate()) {
        fprintf(stderr, "Errors in config!\n");
        return -1;
    }

    // ==============根据配置信息创建 zipformer 的识别器和音频流==============
    sherpa_onnx::OnlineRecognizer recognizer(config);
    auto s = recognizer.CreateStream();

    sherpa_onnx::Microphone mic;

    // ========================== PortAudio 相关参数设置 ===================
    // 使用portaudio枚举和选择音频输入设备
    PaDeviceIndex num_devices = Pa_GetDeviceCount();
    fprintf(stderr, "Num devices: %d\n", num_devices);

    int32_t device_index = Pa_GetDefaultInputDevice();

    if (device_index == paNoDevice) {
        fprintf(stderr, "No default input device found\n");
        fprintf(stderr, "If you are using Linux, please switch to \n");
        fprintf(stderr, " ./bin/sherpa-onnx-alsa \n");
        exit(EXIT_FAILURE);
    }

    const char *pDeviceIndex = std::getenv("SHERPA_ONNX_MIC_DEVICE");
    if (pDeviceIndex) {
        fprintf(stderr, "Use specified device: %s\n", pDeviceIndex);
        device_index = atoi(pDeviceIndex);
    }

    for (int32_t i = 0; i != num_devices; ++i) {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
        fprintf(stderr, " %s %d %s\n", (i == device_index) ? "*" : " ", i, info->name);
    }

    PaStreamParameters param;
    param.device = device_index;

    fprintf(stderr, "Use device: %d\n", param.device);

    const PaDeviceInfo *info = Pa_GetDeviceInfo(param.device);
    fprintf(stderr, "  Name: %s\n", info->name);
    fprintf(stderr, "  Max input channels: %d\n", info->maxInputChannels);

    param.channelCount = 1;
    param.sampleFormat = paFloat32;

    param.suggestedLatency          = info->defaultLowInputLatency;
    param.hostApiSpecificStreamInfo = nullptr;
    const char *pSampleRateStr      = std::getenv("SHERPA_ONNX_MIC_SAMPLE_RATE");
    if (pSampleRateStr) {
        fprintf(stderr, "Use sample rate %f for mic\n", mic_sample_rate);
        mic_sample_rate = static_cast<float>(atof(pSampleRateStr));
    }
    float sample_rate = 16000.0f;

    // 打开并启动音频流
    PaStream *stream;
    PaError err = Pa_OpenStream(&stream, &param,
                                nullptr,  // &outputParameters
                                sample_rate,
                                0,               // frames per buffer
                                paClipOff,       // we won't output out of range samples
                                RecordCallback,  // 注册音频录制回调函数，自动回调，20ms
                                s.get());  // s.get()得到zipformer的音频流标记，作为RecordCallback的最后一个参数传递
    if (err != paNoError) {
        fprintf(stderr, "portaudio error: %s\n", Pa_GetErrorText(err));
        exit(EXIT_FAILURE);
    }

    err = Pa_StartStream(stream);
    fprintf(stderr, "Started 111111111\n");

    if (err != paNoError) {
        fprintf(stderr, "portaudio error: %s\n", Pa_GetErrorText(err));
        exit(EXIT_FAILURE);
    }

    // 主循环：语音识别和处理
    std::string last_text;
    int32_t segment_index = 0;
    sherpa_onnx::Display display(30);

    while (!stop) {
        while (recognizer.IsReady(s.get())) {
            recognizer.DecodeStream(s.get());
        }

        auto text = recognizer.GetResult(s.get()).text;

        /*
            端点：音频信号突变的点，一般表示一句话说完了（静音检测-VAD）
                规则1: 长时间静音2.4s
                规则2: 自然停顿0.4s
                规则3: 最大时长限制20s
        */
        bool is_endpoint = recognizer.IsEndpoint(s.get());

        if (is_endpoint && !config.model_config.paraformer.encoder.empty()) {
            // 为流式Paraformer模型添加尾部填充
            std::vector<float> tail_paddings(static_cast<int>(1.0f * mic_sample_rate));

            s->AcceptWaveform(static_cast<int32_t>(mic_sample_rate), tail_paddings.data(),
                              static_cast<int32_t>(tail_paddings.size()));

            while (recognizer.IsReady(s.get())) {
                recognizer.DecodeStream(s.get());
            }
            // 第二次获取text
            text = recognizer.GetResult(s.get()).text;
        }

        // 实时显示识别结果
        if (!text.empty() && last_text != text) {
            last_text = text;

            display.Print(segment_index, tolowerUnicode(text));
            fflush(stderr);
        }

        if (is_endpoint) {
            if (!text.empty()) {
                // 发送识别结果给LLM处理
                auto response = client.request(text);
                std::cout << "[llm -> voice] received: " << response << '\n';

                // 等待TTS合成完成
                wait                = true;
                auto block_response = block_client.request("block");
                std::cout << "[tts -> voice] received: " << block_response << '\n';
                wait = false;

                ++segment_index;
            }

            // 重置识别器，准备下一轮识别
            recognizer.Reset(s.get());
        }

        Pa_Sleep(20);  // sleep for 20ms
    }

    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "portaudio error: %s\n", Pa_GetErrorText(err));
        exit(EXIT_FAILURE);
    }

    return 0;
}
