#pragma once

#include <rclcpp/rclcpp.hpp>
#include <string>
#include <exception>
#include <vector>
#include <prompt_msgs/msg/prompt.hpp>
#include <prompt_msgs/msg/prompt_response.hpp>


namespace prompt_provider
{

// provider exception class
class PromptProviderException : public std::exception
{
public:
  PromptProviderException(const std::string& msg) : msg_(msg)
  {
  }

  virtual const char* what() const noexcept override
  {
    return msg_.c_str();
  }

private:
  std::string msg_;
};

// class PromptProvider pure virtual
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
  // internal data types
  // prompt option
  struct PromptOption
  {
    std::string key;
    std::string value;
    std::string type;
  };

  // Prompt Conversation struct
  struct PromptDialogue
  {
    std::string role;
    std::string content;
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
    std::vector<PromptOption> options;
    bool success{false};
    double accuracy{0.0};
    double confidence{0.0};
    double risk{0.0};
  };

public:
  PromptProviderBase() = default;
  virtual ~PromptProviderBase() = default;

  // abstract methods
  // init
  virtual void init(rclcpp::node_interfaces::NodeParametersInterface::SharedPtr params,
                    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr log) = 0;

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
      result.options.push_back(PromptOption{ option.key, option.value, option.type });
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

protected:
  rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr logging_;
};

}  // namespace prompt_provider
