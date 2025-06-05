#include <prompt_provider/ollama/single_prompt_provider.hpp>
#include <prompt_provider/ollama/chat_prompt_provider.hpp>
#include <prompt_provider/openai/single_prompt_provider.hpp>
#include <prompt_provider/openai/chat_prompt_provider.hpp>
#include <prompt_base/base_class.hpp>
#include <pluginlib/class_list_macros.hpp>

// plugin
PLUGINLIB_EXPORT_CLASS(prompt::SinglePromptProviderOpenAI, prompt::BaseClass);
PLUGINLIB_EXPORT_CLASS(prompt::SinglePromptProviderOllama, prompt::BaseClass);
PLUGINLIB_EXPORT_CLASS(prompt::ChatPromptProviderOpenAI,   prompt::BaseClass);
PLUGINLIB_EXPORT_CLASS(prompt::ChatPromptProviderOllama,   prompt::BaseClass);
