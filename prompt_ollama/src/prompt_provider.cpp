#include <prompt_base/base_class.hpp>
#include <prompt_ollama/single_provider.hpp>
#include <prompt_ollama/chat_provider.hpp>
#include <pluginlib/class_list_macros.hpp>

// plugin
PLUGINLIB_EXPORT_CLASS(prompt::SingleOllamaProvider, prompt::BaseClass);
PLUGINLIB_EXPORT_CLASS(prompt::ChatOllamaProvider, prompt::BaseClass);
