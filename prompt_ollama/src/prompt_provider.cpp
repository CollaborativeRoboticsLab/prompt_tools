#include <prompt_base/base_class.hpp>
#include <prompt_ollama/ollama_provider.hpp>
#include <pluginlib/class_list_macros.hpp>

// plugin
PLUGINLIB_EXPORT_CLASS(prompt::OllamaProvider, prompt::BaseClass);
