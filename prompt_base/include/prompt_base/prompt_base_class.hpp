#pragma once

#include <prompt_base/rest_base_class.hpp>
#include <prompt_base/utils/prompt_options.hpp>
#include <string>
#include <vector>

namespace prompt
{

/**
 * @brief PromptBaseClass
 *
 * This is the base class for prompt providers that use REST APIs.
 * It provides a common interface for sending prompts and receiving responses.
 */
class PromptBaseClass : public RestBaseClass
{
public:
  PromptBaseClass()
  {
  }

  /**
   * @brief Initialize the PromptBaseClass
   *
   * This method initializes the PromptBaseClass with parameters from the ROS parameter server.
   *
   * @param node rclcpp::Node::SharedPtr ROS nodea word
   * @param plugin_name The name of the plugin to initialize.
   * @param api_key_name The name of the API key parameter (optional).
   */
  virtual void initialize_provider_base(rclcpp::Node::SharedPtr node, std::string plugin_name = "PromptBaseClass",
                                        std::string api_key_name = "")
  {
    // initialize base class
    initialize_rest_base(node, plugin_name, api_key_name);

    // declare parameters
    node_->declare_parameter(plugin_name_ + ".override_model_options", false);

    // get parameters from the parameter server
    override_ = node_->get_parameter(plugin_name_ + ".override_model_options").as_bool();

    if (override_)
    {
      RCLCPP_INFO(node_->get_logger(), "Overriding model options from parameters");
      prompt_options_ = prompt::load_from_paramters(node_, plugin_name_);
    }
    else
    {
      RCLCPP_INFO(node_->get_logger(), "Using model options from prompt request");
    }
  }

  /**
   * @brief sendPrompt send a prompt to a prompt provider using REST
   *
   * Typical providers offer stream based responses which is also supported
   *
   * @param req The prompt request containing the prompt and options.
   * @return const PromptResponse
   */
  virtual prompt::PromptResponse sendPrompt(const prompt::PromptRequest& req) override
  {
    prompt::PromptResponse response;

    // add to the buffer
    prompt_buffer_.push_back(req.prompt);

    if (req.flush)
    {
      // create a string to fill with all buffered prompts
      std::string prompt_cache = "";

      // fill the string with buffered prompts
      for (const auto& prompt_string : prompt_buffer_)
      {
        prompt_cache = prompt_cache + " " + prompt_string + ". ";
      }

      // create a new request
      prompt::PromptRequest request;
      request.prompt = prompt_cache;
      request.options = req.options;

      // check if we should override model options if so do it
      if (override_)
        request.options = prompt_options_;

      // send the prompt request to the prompt_provider
      response = RestBaseClass::sendPrompt(request);
      response.buffered = false;

      prompt_buffer_.clear();
    }
    else
    {
      response.buffered = true;
    }

    return response;
  }

protected:
  /**
   * @brief Override model options
   *
   * This flag indicates whether to override the model options from the parameters or not.
   */
  bool override_;

  /**
   * @brief Model options
   *
   * This is the model options used by the prompt provider.
   */
  std::vector<prompt::PromptOption> prompt_options_;

  /**
   * @brief buffered prompts
   *
   */
  std::vector<std::string> prompt_buffer_;
};

}  // namespace prompt