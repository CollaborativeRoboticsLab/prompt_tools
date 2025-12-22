#pragma once
#include <string>
#include <vector>

namespace prompt
{
    // prompt option
    struct PromptOption
    {
        std::string key;
        std::string value;
        std::string type;
    };

    // Prompt Conversation struct
    struct PromptDialogue
    {
        std::string role;
        std::string content;
    };

    // prompt provider request
    struct PromptRequest
    {
        std::string prompt;
        bool use_cache;
        bool flush_cache;
        bool use_chat_mode;
        std::string model_family;
        std::vector<PromptOption> options;
    };

    // prompt provider response
    struct PromptResponse
    {
        std::string response;
        bool buffered;
        std::vector<PromptOption> options;
        bool success{false};
        double accuracy{0.0};
        double confidence{0.0};
        double risk{0.0};
    };
}