#include <prompt_provider/rest/single_prompt_provider_openai.hpp>
#include <prompt_provider/rest/single_prompt_provider_ollama.hpp>
#include <prompt_provider/rest/chat_prompt_provider_openai.hpp>
#include <prompt_provider/rest/chat_prompt_provider_ollama.hpp>
#include <pluginlib/class_list_macros.hpp>

namespace prompt_provider
{

}  // namespace prompt_provider

// plugin
PLUGINLIB_EXPORT_CLASS(prompt_provider::rest::SinglePromptProviderOpenAI, prompt_provider::PromptProviderBase);
PLUGINLIB_EXPORT_CLASS(prompt_provider::rest::SinglePromptProviderOllama, prompt_provider::PromptProviderBase);
PLUGINLIB_EXPORT_CLASS(prompt_provider::rest::ChatPromptProviderOpenAI, prompt_provider::PromptProviderBase);
PLUGINLIB_EXPORT_CLASS(prompt_provider::rest::ChatPromptProviderOllama, prompt_provider::PromptProviderBase);
