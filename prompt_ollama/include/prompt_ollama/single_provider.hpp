#pragma once

#include <prompt_base/prompt_base_class.hpp>

namespace prompt
{

/**
 * @brief SingleOllamaProvider
 *
 * This is a prompt provider that uses a REST API to send and receive prompts
 * the typical rest api uses application/json content type so that is what is
 * supported
 *
 */
class SingleOllamaProvider : public PromptBaseClass
{
public:
  /**
   * @brief Construct a new Single Prompt Provider Ollama object
   *
   */
  SingleOllamaProvider() : PromptBaseClass()
  {
  }

  /**
   * @brief Initialize the SingleOllamaProvider
   *
   * This method initializes the SingleOllamaProvider with parameters from the ROS parameter server.
   *
   * @param node rclcpp::Node::SharedPtr ROS node
   */
  virtual void initialize(rclcpp::Node::SharedPtr node) override
  {
    // initialize base class
    initialize_provider_base(node, "SingleOllamaProvider", "");
  }

protected:
  virtual Poco::JSON::Object toJson(const prompt::PromptRequest& prompt) override
  {
    // add options
    Poco::JSON::Object result = handle_options(prompt);

    // add prompt
    result.set("prompt", prompt.prompt);

    return result;
  }

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
