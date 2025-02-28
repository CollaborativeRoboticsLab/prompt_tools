#pragma once

#include <prompt_scheme/scheme_base.hpp>
#include <prompt_scheme/tools/structs.hpp>
#include <prompt_scheme/tools/xml_scrubber.hpp>
#include <prompt_utils/prompt_options.hpp>
#include <prompt_utils/structs.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>
#include <vector>

namespace prompt_scheme
{
/**
 * @brief BufferScheme
 *
 * prompt scheme for the behaviour tree xml format
 * a BT is created by conversation between prompt bridge and language model
 * once a BT document is finalised, it is actioned onboard the robot
 * this scheme is used when a robot is requisitioned for a complex goal
 * and a language model is used to guide the robot to achieve that goal
 *
 */
class BufferScheme : public SchemeBase
{
public:
  BufferScheme() = default;

  virtual void init(rclcpp::node_interfaces::NodeParametersInterface::SharedPtr params,
                    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr log) override
  {
    logging_ = log;

    prompt_options_ = prompt::load_from_paramters(params, "BufferScheme");

    override_ =
        params->declare_parameter("BufferScheme.override_model_options", rclcpp::ParameterValue(false)).get<bool>();
  }

  virtual prompt::PromptResponse processPrompt(const prompt::PromptRequest& req) override
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
        prompt_cache = prompt_cache + " "  + prompt_string + ". " ;
      }

      // create a new request
      prompt::PromptRequest request;
      request.prompt = prompt_cache;
      request.options = prompt_options_;

      // send the prompt request to the prompt_provider
      response = prompt_provider_->sendPrompt(request);
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
  bool starting(const std::string& prompt) override
  {
    return true;
  }

  bool collecting(const std::string& prompt) override
  {
    return true;
  }

  bool negotiating(const std::string& prompt) override
  {
    return true;
  }

  bool running(const std::string& prompt) override
  {
    return true;
  }

private:
  // override model options from the ros messages
  bool override_;

  // model options
  std::vector<prompt::PromptOption> prompt_options_;

  // buffered prompts
  std::vector<std::string> prompt_buffer_;
};

}  // namespace prompt_scheme
