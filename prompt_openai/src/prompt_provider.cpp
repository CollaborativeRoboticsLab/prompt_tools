#include <prompt_base/base_class.hpp>
#include <prompt_openai/single_provider.hpp>
#include <prompt_openai/chat_provider.hpp>
#include <pluginlib/class_list_macros.hpp>

// plugin
PLUGINLIB_EXPORT_CLASS(prompt::SingleOpenAIProvider, prompt::BaseClass);
PLUGINLIB_EXPORT_CLASS(prompt::ChatOpenAIProvider,   prompt::BaseClass);
