#pragma once

#include <rclcpp/rclcpp.hpp>
#include <prompt_msgs/msg/prompt.hpp>
#include <prompt_msgs/msg/prompt_response.hpp>

#include <pluginlib/class_list_macros.hpp>

namespace prompt_provider
{

// class PromptProvider pure virtual
class PromptProvider
{
public:
  // internal data types
  // prompt option
  struct PromptOption
  {
    std::string key;
    std::string value;
  };

  // prompt provider request
  struct PromptRequest
  {
    std::string prompt;
    std::vector<PromptOption> options;
  };

  // prompt provider response
  struct PromptResponse
  {
    std::string response;
    bool success;
    double accuracy;
    double confidence;
    double risk;
  };

public:
  // cannot instantiate base class
  PromptProvider() = delete;

  // destructor
  virtual ~PromptProvider() = default;

  // abstract methods
  // sendPrompt
  virtual const PromptResponse sendPrompt(const PromptRequest& req) = 0;

public:
  // static methods for message conversions
  // fromMsg
  static const PromptRequest fromMsg(const prompt_msgs::msg::Prompt& prompt)
  {
    PromptRequest result;
    result.prompt = prompt.prompt;
    for (const auto& option : prompt.options)
    {
      result.options.push_back(PromptOption{ option.key, option.value });
    }
    return result;
  }

  // toMsg
  static const prompt_msgs::msg::PromptResponse toMsg(const PromptResponse& res)
  {
    prompt_msgs::msg::PromptResponse result;
    result.response = res.response;
    result.success = res.success;
    result.accuracy = res.accuracy;
    result.confidence = res.confidence;
    result.risk = res.risk;
    return result;
  }
};
}  // namespace prompt_provider
