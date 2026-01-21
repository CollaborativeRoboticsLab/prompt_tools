#include <pluginlib/class_list_macros.hpp>
#include <prompt_base/base_class.hpp>
#include <prompt_openai/openai_provider.hpp>
#include <prompt_openai/openai_embedding.hpp>
#include <prompt_openai/openai_tokenize.hpp>

// plugin
PLUGINLIB_EXPORT_CLASS(prompt::OpenAIProvider, prompt::BaseClass);
PLUGINLIB_EXPORT_CLASS(prompt::OpenAIEmbedding, prompt::BaseClass);
PLUGINLIB_EXPORT_CLASS(prompt::OpenAITokenize, prompt::BaseClass);
