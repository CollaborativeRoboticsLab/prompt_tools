#include <prompt_base/base_class.hpp>
#include <prompt_transcriber/openai_whisper.hpp>
#include <pluginlib/class_list_macros.hpp>

// plugin
PLUGINLIB_EXPORT_CLASS(prompt::WhisperAPI, prompt::BaseClass);