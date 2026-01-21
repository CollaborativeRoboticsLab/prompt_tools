#pragma once
#include <prompt_base/utils/structs.hpp>
#include <prompt_msgs/msg/model_option.hpp>
#include <prompt_msgs/msg/embed.hpp>
#include <prompt_msgs/msg/embed_response.hpp>
#include <prompt_msgs/msg/prompt.hpp>
#include <prompt_msgs/msg/prompt_response.hpp>
#include <prompt_msgs/msg/token.hpp>
#include <prompt_msgs/msg/token_response.hpp>

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
 * @return const prompt::EmbedRequest
 */
static const prompt::EmbedRequest fromMsg(const prompt_msgs::msg::Embed& input)
{
  prompt::EmbedRequest result;

  result.text = input.text;
  result.model_family = input.model_family;

  for (const auto& option : input.options)
  {
    result.options.push_back(prompt::PromptOption{ option.key, option.value, option.type });
  }
  return result;
}

/**
 * @brief Converts prompt_msgs::msg::Token into prompt::TokenRequest
 * which is used internally in prompt tools
 *
 * @param prompt input ros2 message
 * @return const prompt::TokenRequest
 */
static const prompt::TokenRequest fromMsg(const prompt_msgs::msg::Token& input)
{
  prompt::TokenRequest result;

  result.text = input.text;
  result.tokens = input.tokens;
  result.encode = input.encode;
  result.model_family = input.model_family;

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

  for (const auto& emb : res.embeddings)
  {
    prompt_msgs::msg::Embedding embedding_msg;
    if (emb.is_float)
    {
      embedding_msg.float_embedding = emb.float_embedding;
    }
    else
    {
      embedding_msg.base64_embedding = emb.base64_embedding;
    }
    embedding_msg.is_float = emb.is_float;
    embedding_msg.index = emb.index;
    result.embeddings.push_back(embedding_msg);
  }

  result.success = res.success;
  result.error = res.error;
  result.model = res.model;
  result.prompt_tokens = res.prompt_tokens;
  result.total_tokens = res.total_tokens;

  return result;
}

/**
 * @brief Converts prompt::TokenResponse into prompt_msgs::msg::TokenResponse
 * which is used in ros2 eco system
 *
 * @param res
 * @return const prompt_msgs::msg::TokenResponse
 */
static const prompt_msgs::msg::TokenResponse toMsg(const prompt::TokenResponse& res)
{
  prompt_msgs::msg::TokenResponse result;
  result.tokens = res.tokens;
  result.text = res.text;
  result.success = res.success;
  result.error = res.error;
  return result;
}

}  // namespace prompt