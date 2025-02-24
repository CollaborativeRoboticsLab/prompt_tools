#pragma once

#include <rclcpp/rclcpp.hpp>
#include <prompt_msgs/msg/prompt.hpp>
#include <prompt_msgs/msg/prompt_response.hpp>
#include <prompt_utils/structs.hpp>
#include <prompt_utils/exceptions.hpp>

namespace prompt_provider
{
/**
 * @brief PromptProviderBase
 *
 * This is the base class for prompt provider plugins
 * it provides a common interface for all prompt provider plugins
 * the main functions are to send a prompt and recieve a response
 *
 */
class PromptProviderBase
{
public:

public:
  PromptProviderBase() = default;
  virtual ~PromptProviderBase() = default;

  // abstract methods
  // init
  virtual void init(rclcpp::node_interfaces::NodeParametersInterface::SharedPtr params,
                    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr log) = 0;

  // sendPrompt
  virtual const prompt::PromptResponse sendPrompt(const prompt::PromptRequest& req) = 0;

public:
  // static methods for message conversions
  // fromMsg
  static const prompt::PromptRequest fromMsg(const prompt_msgs::msg::Prompt& prompt)
  {
    prompt::PromptRequest result;

    result.prompt = prompt.prompt;
    for (const auto& option : prompt.options)
    {
      result.options.push_back(prompt::PromptOption{ option.key, option.value, option.type });
    }
    return result;
  }

  // toMsg
  static const prompt_msgs::msg::PromptResponse toMsg(const prompt::PromptResponse& res)
  {
    prompt_msgs::msg::PromptResponse result;
    result.response = res.response;
    result.success = res.success;
    result.accuracy = res.accuracy;
    result.confidence = res.confidence;
    result.risk = res.risk;
    return result;
  }

protected:
  rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr logging_;
};

}  // namespace prompt_provider
