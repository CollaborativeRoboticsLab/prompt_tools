#pragma once
#include <prompt_msgs/msg/prompt.hpp>
#include <prompt_msgs/msg/prompt_response.hpp>
#include <prompt_base/utils/structs.hpp>

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
  result.flush  = prompt.flush;

  for (const auto& option : prompt.options)
  {
    result.options.push_back(prompt::PromptOption{ option.key, option.value, option.type });
  }
  return result;
}

/**
 * @brief Converts prompt::PromptResponse prompt_msgs::msg::PromptResponse  
 * into which is used in ros2 eco system
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

}  // namespace prompt