#include <prompt_base/base_class.hpp>
#include <prompt_huggingface/sentiment_analysis.hpp>
#include <pluginlib/class_list_macros.hpp>

// plugin
PLUGINLIB_EXPORT_CLASS(prompt::SentimentAnalysis, prompt::BaseClass);