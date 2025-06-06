#pragma once

#include <rclcpp/rclcpp.hpp>
#include <prompt_base/utils/exceptions.hpp>
#include <prompt_base/utils/structs.hpp>

namespace prompt
{
/**
 * @brief PromptProviderBase
 *
 * This is the base class for prompt plugins it provides a common interface for all prompt 
 * plugins where the main functions are to send a prompt and recieve a response
 *
 */
class BaseClass
{
  
public:
  BaseClass() = default;
  virtual ~BaseClass() = default;

  /**
   * @brief Initialize the prompt pligin
   *
   * This method initializes the prompt plugin with parameters from the ROS parameter server.
   *
   * @param node rclcpp::Node::SharedPtr ROS node
   */
  virtual void initialize(rclcpp::Node::SharedPtr node) = 0;

  /**
   * @brief Initialize the prompt base class
   *
   * This method initializes the baseclass with parameters from the ROS parameter server.
   * 
   * @param node rclcpp::Node::SharedPtr ROS node
   */
  virtual void initialize_base(rclcpp::Node::SharedPtr node, std::string plugin_name = "BaseClass")
  {
    // set the node pointer
    node_ = node;

    // Declare the plugin name parameter
    plugin_name_ = plugin_name;

    // Log the initialization
    RCLCPP_INFO(node_->get_logger(), "BaseClass initialized for plugin: %s", plugin_name_.c_str());
  }

  /**
   * @brief Send a prompt to the prompt plugin
   *
   * This method sends a prompt request to the prompt plugin and returns a response.
   *
   * @param req The prompt request containing the prompt and options.
   * @return The response from the prompt provider.
   * @throws prompt::PromptException if the method is not implemented in the derived class.
   */
  virtual prompt::PromptResponse sendPrompt(const prompt::PromptRequest& req)
  {
    throw prompt::PromptException("sendPrompt not implemented in base class");
  }

  /**
   * @brief Send a prompt with a file to the prompt plugin
   * 
   * This method sends a prompt request with a file to the prompt plugin and returns a response.
   * 
   * @param req The prompt request containing the prompt, options, and file.
   * @return prompt::PromptResponse from the prompt provider.
   * @throws prompt::PromptException if the method is not implemented in the derived class.
   */
  virtual prompt::PromptResponse sendPromptAudio(const prompt::PromptRequest& req)
  {
    throw prompt::PromptException("sendPromptAudio not implemented in base class");
  }

protected:
  /**
   * @brief Node shared pointer
   *
   * This is the shared pointer to the ROS node used by the prompt plugin.
   */
  rclcpp::Node::SharedPtr node_;

  /**
   * @brief Plugin name
   *
   * This is the name of the prompt plugin.
   */
  std::string plugin_name_;
};

}  // namespace prompt
