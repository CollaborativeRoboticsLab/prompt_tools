#pragma once

#include <rclcpp/rclcpp.hpp>
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
  virtual prompt::PromptResponse sendPrompt(const prompt::PromptRequest& req) = 0;

protected:
  rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr logging_;
};

}  // namespace prompt_provider
