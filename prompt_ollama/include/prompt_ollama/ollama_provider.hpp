#pragma once

#include <prompt_base/prompt_base_class.hpp>

namespace prompt
{

/**
 * @brief OllamaProvider
 *
 * This is a prompt provider that uses a REST API to send and receive prompts
 * the typical rest api uses application/json content type so that is what is
 * supported
 *
 */
class OllamaProvider : public PromptBaseClass
{
public:
  /**
   * @brief Construct a new OllamaProvider object
   *
   */
  OllamaProvider() : PromptBaseClass()
  {
  }

  /**
   * @brief Initialize the OllamaProvider
   *
   * This method initializes the OllamaProvider with parameters from the ROS parameter server.
   *
   * @param node rclcpp::Node::SharedPtr ROS node
   */
  virtual void initialize(rclcpp::Node::SharedPtr node) override
  {
    // initialize base class
    initialize_prompt_base(node, "OllamaProvider", "");
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
  virtual Poco::JSON::Object toJson(prompt::PromptRequest& prompt) override
  {
    // add options
    Poco::JSON::Object result = handle_options(prompt.options);

    // add prompt
    result.set("prompt", prompt.prompt);

    return result;
  }

  /**
   * @brief Convert a prompt request with conversation history to a JSON object
   *
   * This method converts the prompt request and conversation history to a JSON object that can be sent to the prompt
   * plugin. It includes the prompt text, options, and conversation history in the JSON object.
   *
   * @param prompt The prompt request to convert
   * @param conversation The conversation history
   * @return A JSON object representing the prompt request with conversation history
   */
  virtual Poco::JSON::Object toJsonConversation(prompt::PromptRequest& prompt,
                                                std::vector<PromptDialogue>& conversation)
  {
    // add options
    Poco::JSON::Object result = handle_options(prompt.options);

    Poco::JSON::Array messages_array;

    for (const prompt::PromptDialogue& dialog_ : conversation_)
    {
      Poco::JSON::Object dialog_object_;

      dialog_object_.set("role", dialog_.role);
      dialog_object_.set("content", dialog_.content);

      messages_array.add(dialog_object_);
    }

    // add latest prompt
    Poco::JSON::Object latest_dialog_object_;
    latest_dialog_object_.set("role", "user");
    latest_dialog_object_.set("content", prompt.prompt);

    messages_array.add(latest_dialog_object_);

    // add prompt
    result.set("messages", messages_array);

    std::ostringstream jsonStream;
    result.stringify(jsonStream);
    RCLCPP_INFO(node_->get_logger(), "Poco JSON Object: %s", jsonStream.str().c_str());

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

  /**
   * @brief Convert a JSON object to a prompt response with conversation history
   *
   * This method converts a JSON object received from the prompt plugin into a prompt response,
   * taking into account the conversation history.
   * It extracts the relevant fields from the JSON object and returns a PromptResponse object.
   *
   * @param object The JSON object to convert
   * @return A PromptResponse object containing the response data
   */
  virtual prompt::PromptResponse fromJsonConversation(const Poco::JSON::Object::Ptr object)
  {
    prompt::PromptResponse res;

    // TODO: create custom parsers for specific options from different apis
    // res.success = object->get("success").convert<bool>();
    // res.accuracy = object->get("accuracy").convert<double>();
    // res.confidence = object->get("confidence").convert<double>();
    // res.risk = object->get("risk").convert<double>();
    // for all other variables loop and push back to response key/values

    for (Poco::JSON::Object::ConstIterator it = object->begin(); it != object->end(); ++it)
    {
      if (it->first != "message")
      {
        res.options.push_back(prompt::PromptOption{ it->first, it->second.convert<std::string>(), "" });
      }
      else
      {
        Poco::JSON::Object::Ptr messageObj = object->getObject("message");
        res.response = messageObj->get("content").toString();
      }
    }

    return res;
  }

  /**
   * @brief A vector of prompt dialogues representing the conversation history.
   *
   * This vector stores the sequence of messages exchanged in the chat, including both user and assistant messages.
   */
  std::vector<prompt::PromptDialogue> conversation_;
};

}  // namespace prompt
