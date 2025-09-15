#pragma once

#include <prompt_base/prompt_base_class.hpp>

namespace prompt
{

/**
 * @brief SingleOpenAIProvider
 *
 * This is a prompt provider that uses a REST API to send and receive prompts
 * the typical rest api uses application/json content type so that is what is
 * supported
 *
 */
class SingleOpenAIProvider : public PromptBaseClass
{
public:
  /**
   * @brief Construct a new Single Prompt Provider OpenAI object
   *
   */
  SingleOpenAIProvider() : PromptBaseClass()
  {
  }

  /**
   * @brief Initialize the SingleOpenAIProvider
   *
   * This method initializes the SingleOpenAIProvider with parameters from the ROS parameter server.
   *
   * @param node rclcpp::Node::SharedPtr ROS node
   */
  virtual void initialize(rclcpp::Node::SharedPtr node) override
  {
    // initialize base class
    initialize_provider_base(node, "SingleOpenAIProvider", "OPENAI_API_KEY");
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
   * @brief Convert a prompt request to a JSON object
   *
   * This method converts the prompt request to a JSON object that can be sent to the prompt plugin.
   * It includes the prompt text and options in the JSON object.
   *
   * @param prompt The prompt request to convert
   * @return A JSON object representing the prompt request
   */
  virtual Poco::JSON::Object toJson(const prompt::PromptRequest& prompt) override
  {
    // add options
    Poco::JSON::Object result = handle_options(prompt);

    // add prompt
    result.set("prompt", prompt.prompt);

    return result;
  }

  /**
   * @brief Convert a JSON object to a prompt response
   *
   * This method converts a JSON object received from the prompt plugin into a PromptResponse.
   * It extracts the response content and options from the JSON object.
   *
   * @param object The JSON object to convert
   * @return A PromptResponse containing the response content and options
   */
  virtual prompt::PromptResponse fromJson(const Poco::JSON::Object::Ptr object) override
  {
    prompt::PromptResponse res;

    // try parse response
    if (object->get("response"))
      res.response = object->get("response").toString();

    // TODO: create custom parsers for specific options from different apis
    // res.success = object->get("success").convert<bool>();
    // res.accuracy = object->get("accuracy").convert<double>();
    // res.confidence = object->get("confidence").convert<double>();
    // res.risk = object->get("risk").convert<double>();
    // for all other variables loop and push back to response key/values
    for (Poco::JSON::Object::ConstIterator it = object->begin(); it != object->end(); ++it)
    {
      res.options.push_back(prompt::PromptOption{ it->first, it->second.convert<std::string>(), "" });
    }

    return res;
  }
};

}  // namespace prompt
