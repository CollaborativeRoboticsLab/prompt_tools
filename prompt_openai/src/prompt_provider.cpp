#include <prompt_base/base_class.hpp>
#include <prompt_openai/openai_provider.hpp>
#include <pluginlib/class_list_macros.hpp>

// plugin
PLUGINLIB_EXPORT_CLASS(prompt::OpenAIProvider, prompt::BaseClass);
