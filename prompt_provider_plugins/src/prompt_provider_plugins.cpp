#include <prompt_provider_plugins/rest/single_prompt_provider.hpp>
#include <prompt_provider_plugins/rest/chat_prompt_provider.hpp>
#include <pluginlib/class_list_macros.hpp>

namespace prompt_provider
{

}  // namespace prompt_provider

// plugin
PLUGINLIB_EXPORT_CLASS(prompt_provider::rest::SinglePromptProvider, prompt_provider::PromptProviderBase)
PLUGINLIB_EXPORT_CLASS(prompt_provider::rest::ChatPromptProvider, prompt_provider::PromptProviderBase)
