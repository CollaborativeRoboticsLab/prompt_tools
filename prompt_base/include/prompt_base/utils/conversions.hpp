#pragma once
#include <prompt_base/utils/structs.hpp>
#include <prompt_msgs/msg/prompt.hpp>
#include <prompt_msgs/msg/prompt_response.hpp>

namespace prompt
{

/**
 * @brief Converts prompt_msgs::msg::Prompt into prompt::PromptRequest
 * which is used internally in prompt tools
 *
 * @param prompt input ros2 message
 * @return const prompt::PromptRequest
 */
static const prompt::PromptRequest fromMsg(const prompt_msgs::msg::Prompt& prompt)
{
  prompt::PromptRequest result;

  result.prompt = prompt.prompt;
  result.use_cache = prompt.use_cache;
  result.flush_cache = prompt.flush_cache;
  result.use_chat_mode = prompt.use_chat_mode;
  result.model_family = prompt.model_family;

  for (const auto& option : prompt.options)
  {
    result.options.push_back(prompt::PromptOption{ option.key, option.value, option.type });
  }
  return result;
}

/**
 * @brief Converts prompt_msgs::msg::Embed into prompt::EmbedRequest
 * which is used internally in prompt tools
 *
 * @param prompt input ros2 message
 * @return const prompt::PromptRequest
 */
static const prompt::EmbedRequest fromMsg(const prompt_msgs::msg::Embed& input)
{
  prompt::EmbedRequest result;

  result.text = input.text;
  result.model_family = input.model_family;
  result.embed_type = static_cast<prompt::EmbedType>(input.embed_type);

  if (input.embed_type == prompt_msgs::msg::EmbedFormat::FLOAT)
  {
    result.embed_type = prompt::EmbedType::Float;
  }
  else if (input.embed_type == prompt_msgs::msg::EmbedFormat::BASE64)
  {
    result.embed_type = prompt::EmbedType::Base64;
  }
  else
  {
    throw prompt::PromptException("Invalid embed_type in Embed message");
  }

  for (const auto& option : input.options)
  {
    result.options.push_back(prompt::PromptOption{ option.key, option.value, option.type });
  }
  return result;
}

/**
 * @brief Converts prompt::PromptResponse into prompt_msgs::msg::PromptResponse
 * which is used in ros2 eco system
 *
 * @param res
 * @return const prompt_msgs::msg::PromptResponse
 */
static const prompt_msgs::msg::PromptResponse toMsg(const prompt::PromptResponse& res)
{
  prompt_msgs::msg::PromptResponse result;
  result.response = res.response;
  result.buffered = res.buffered;
  result.success = res.success;
  result.accuracy = res.accuracy;
  result.confidence = res.confidence;
  result.risk = res.risk;
  return result;
}

/**
 * @brief Converts prompt::EmbedResponse into prompt_msgs::msg::EmbedResponse
 * which is used in ros2 eco system
 *
 * @param res
 * @return const prompt_msgs::msg::EmbedResponse
 */
static const prompt_msgs::msg::EmbedResponse toMsg(const prompt::EmbedResponse& res)
{
  prompt_msgs::msg::EmbedResponse result;

  if (EmbedType::Float == res.embed_type)
  {
    result.format = prompt_msgs::msg::EmbedFormat::FLOAT;
    result.float_embedding = res.float_embedding;
  }
  else if (EmbedType::Base64 == res.embed_type)
  {
    result.format = prompt_msgs::msg::EmbedFormat::BASE64;
    result.base64_embedding = res.base64_embedding;
  }

  for (const auto& option : res.options)
  {
    prompt_msgs::msg::PromptOption msg_option;
    msg_option.key = option.key;
    msg_option.value = option.value;
    msg_option.type = option.type;
    result.options.push_back(msg_option);
  }

  result.error = res.error;
  result.success = res.success;

  return result;
}

}  // namespace prompt