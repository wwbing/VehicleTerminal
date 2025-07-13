// Copyright (c) 2024 by Rockchip Electronics Co., Ltd. All Rights Reserved.
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

#include <string.h>
#include <unistd.h>
#include <string>
#include "rkllm.h"
#include <fstream>
#include <iostream>
#include <csignal>
#include <vector>
#include <set>
#include "ZmqServer.h"
#include "ZmqClient.h"
#include <cwchar>
#include <locale>
#include <clocale>
#include <cstdlib>
#include <codecvt>

using namespace std;
LLMHandle llmHandle = nullptr;

zmq_component::ZmqServer server;
zmq_component::ZmqClient client("tcp://localhost:7777");

std::wstring buffer_; 
static const std::set<wchar_t> split_chars = {
    L'：',   
    L'，',   
    L'。',  
    L'\n',   
    L'；',  
    L'！',  
    L'？'   
};

bool is_valid_utf8_continuation(uint8_t c)
{
    return (c & 0xC0) == 0x80; 
}

// std::string extract_after_think(const std::string &input)
// {
//     const std::string start_tag = "<think>";
//     const std::string end_tag = "</think>";

//     size_t start_pos = input.find(start_tag);
//     size_t end_pos = input.find(end_tag);

//     std::string result;

//     if (start_pos != std::string::npos && end_pos != std::string::npos && end_pos > start_pos)
//     {
//         result = input.substr(start_pos + start_tag.length(), end_pos - start_pos - start_tag.length());
//     } else if (end_pos != std::string::npos) {
//         result = input.substr(end_pos + end_tag.length());
//     } else {
//         result = input;
//     }

//     // 定义要过滤的字符
//     const std::string punct = " \t\n\r*#@$%^&，。：、；！？【】（）“”‘’";

//     // 安全过滤：保护 UTF-8 多字节字符
//     std::string filtered;
//     for (size_t i = 0; i < result.size();)
//     {
//         uint8_t c = result[i];
//         if (punct.find(c) != std::string::npos)
//         {
//             i++;
//         }
//         else
//         {
//             size_t char_len = 1;
//             if ((c & 0xE0) == 0xC0)
//                 char_len = 2;
//             else if ((c & 0xF0) == 0xE0)
//                 char_len = 3;
//             else if ((c & 0xF8) == 0xF0)
//                 char_len = 4;

//             bool is_valid = true;
//             for (size_t j = 1; j < char_len; ++j)
//             {
//                 if (i + j >= result.size() || !is_valid_utf8_continuation(result[i + j]))
//                 {
//                     is_valid = false;
//                     break;
//                 }
//             }

//             if (is_valid)
//             {
//                 filtered.append(result.substr(i, char_len));
//                 i += char_len;
//             }
//             else
//             {
//                 i++;
//             }
//         }
//     }

//     return filtered;
// }

std::wstring extract_after_think(const std::wstring &input)
{
    const std::wstring start_tag = L"<think>";
    const std::wstring end_tag = L"</think>";

    size_t start_pos = input.find(start_tag);
    size_t end_pos = input.find(end_tag);

    std::wstring result;

    if (start_pos != std::wstring::npos && end_pos != std::wstring::npos && end_pos > start_pos)
    {
        result = input.substr(start_pos + start_tag.length(), end_pos - start_pos - start_tag.length());
    }
    else if (end_pos != std::wstring::npos)
    {
        result = input.substr(end_pos + end_tag.length());
    }
    else
    {
        result = input;
    }

    // 定义要过滤的字符（宽字符版本）
    const std::wstring punct = L" \t\n\r*#@$%^&，。：、；！？【】（）“”‘’";

    // 过滤处理
    std::wstring filtered;
    for (wchar_t c : result)
    {
        if (punct.find(c) == std::wstring::npos)
        {
            filtered += c;
        }
    }

    return filtered;
}

std::wstring utf8_to_wstring(const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string wstring_to_utf8(const std::wstring &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(str);
}
void exit_handler(int signal)
{
    if (llmHandle != nullptr)
    {
        {
            cout << "程序即将退出" << endl;
            LLMHandle _tmp = llmHandle;
            llmHandle = nullptr;
            rkllm_destroy(_tmp);
        }
    }
    exit(signal);
}

void send_response(const std::wstring& text) {
    // std::string response_str = extract_after_think(wstring_to_utf8(text));
    std::string response_str = wstring_to_utf8(extract_after_think(text));
    auto response = client.request(response_str);
    std::cout << "[tts -> llm] received : " << response << std::endl;
}

void callback(RKLLMResult *result, void *userdata, LLMCallState state)
{

    if (state == RKLLM_RUN_FINISH)
    {
        if (!buffer_.empty())
        {
            std::string response_str = wstring_to_utf8(extract_after_think(buffer_)) + " END";
            auto response = client.request(response_str);
            std::cout << "[tts -> llm] received: " << response << std::endl;
            buffer_.clear();
        }
        else
        {
            auto response = client.request("END");
            std::cout << "[tts -> llm] received: " << response << std::endl;
        }

        printf("\n");
    }
    else if (state == RKLLM_RUN_ERROR)
    {
        printf("\\run error\n");
    }
    else if (state == RKLLM_RUN_NORMAL)
    {
        /* ================================================================================================================
        若使用GET_LAST_HIDDEN_LAYER功能,callback接口会回传内存指针:last_hidden_layer,token数量:num_tokens与隐藏层大小:embd_size
        通过这三个参数可以取得last_hidden_layer中的数据
        注:需要在当前callback中获取,若未及时获取,下一次callback会将该指针释放
        ===============================================================================================================*/
        if (result->last_hidden_layer.embd_size != 0 && result->last_hidden_layer.num_tokens != 0)
        {
            int data_size = result->last_hidden_layer.embd_size * result->last_hidden_layer.num_tokens * sizeof(float);
            printf("\ndata_size:%d", data_size);
            std::ofstream outFile("last_hidden_layer.bin", std::ios::binary);
            if (outFile.is_open())
            {
                outFile.write(reinterpret_cast<const char *>(result->last_hidden_layer.hidden_states), data_size);
                outFile.close();
                std::cout << "Data saved to output.bin successfully!" << std::endl;
            }
            else
            {
                std::cerr << "Failed to open the file for writing!" << std::endl;
            }
        }

        printf("%s", result->text);

        // std::wstring wide_text = utf8_to_wstring(result->text);
        // buffer_ += wide_text;

        // if (split_chars.count(wide_text))
        // {
        //     std::cout << "split_chars"<< wstring_to_utf8(wide_text)<<std::endl;
        //     if (!buffer_.empty())
        //     {
        //         auto response = client.request(wstring_to_utf8(buffer_));
        //         std::cout << "Client received: " << response << std::endl;

        //         buffer_.clear();
        //     }
        // }
         std::wstring wide_text = utf8_to_wstring(result->text);
        
        for (wchar_t c : wide_text) {
            buffer_ += c;

            if (split_chars.count(c)) {
                if (!buffer_.empty()) {
                    send_response(buffer_);
                    buffer_.clear();
                }
            }
        }
    }
}

void Init(const string &model_path)
{
    
    RKLLMParam param = rkllm_createDefaultParam();
    param.model_path = model_path.c_str();

    param.top_k = 1;
    param.top_p = 0.95;
    param.temperature = 0.8;
    param.repeat_penalty = 1.1;
    param.frequency_penalty = 0.0;
    param.presence_penalty = 0.0;

    param.max_new_tokens = 100;
    param.max_context_len = 256;
    param.skip_special_token = true;
    param.extend_param.base_domain_id = 0;
    param.extend_param.embed_flash = 1;
    param.extend_param.enabled_cpus_num = 2;
    param.extend_param.enabled_cpus_mask = CPU0 | CPU2;

    int ret = rkllm_init(&llmHandle, &param, callback);
    if (ret == 0)
    {
        printf("rkllm init success\n");
    }
    else
    {
        printf("rkllm init failed\n");
        exit_handler(-1);
    }
}

void receive_asr_data_and_process()
{
    RKLLMInferParam rkllm_infer_params;
    memset(&rkllm_infer_params, 0, sizeof(RKLLMInferParam)); 

    rkllm_infer_params.mode = RKLLM_INFER_GENERATE;

    rkllm_infer_params.keep_history = 0;
    rkllm_set_chat_template(llmHandle, "", "<｜User｜>", "<｜Assistant｜>");

    RKLLMInput rkllm_input;

    while (true)
    {
        std::string input_str;

        rkllm_input.input_type = RKLLM_INPUT_PROMPT;
        input_str = server.receive();
        std::cout << "[voice -> llm] received: " << input_str << std::endl;
        server.send("llm sucess reply !!!");
        rkllm_input.prompt_input = (char *)input_str.c_str();

        // 若要使用普通推理功能,则配置rkllm_infer_mode为RKLLM_INFER_GENERATE或不配置参数
        rkllm_run(llmHandle, &rkllm_input, &rkllm_infer_params, NULL);
    }
}

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "en_US.UTF-8"); 

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " model_path\n";
        return 1;
    }

    signal(SIGINT, exit_handler);
    printf("rkllm init start\n");

    Init(argv[1]);

    receive_asr_data_and_process();

    rkllm_destroy(llmHandle);

    return 0;
}