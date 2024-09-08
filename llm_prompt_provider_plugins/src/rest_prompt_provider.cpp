#include <llm_prompt_provider_plugins/rest_prompt_provider.hpp>
#include <pluginlib/class_list_macros.hpp>

namespace prompt_provider
{

}  // namespace prompt_provider

// plugin
PLUGINLIB_EXPORT_CLASS(prompt_provider::RestPromptProvider,
                       prompt_provider::PromptProviderBase)
