#pragma once

#include <prompt_base/prompt_base_class.hpp>

namespace prompt
{

/**
 * @brief ChatOllamaProvider
 *
 * This is a prompt provider that uses a REST API to send and receive prompts
 * the typical rest api uses application/json content type so that is what is
 * supported
 *
 */
class ChatOllamaProvider : public PromptBaseClass
{
public:
  /**
   * @brief Construct a new Chat Prompt Provider Ollama object
   *
   */
  ChatOllamaProvider()
  {
  }

  /**
   * @brief Initialize the ChatOllamaProvider
   *
   * This method initializes the ChatOllamaProvider with parameters from the ROS parameter server.
   *
   * @param node rclcpp::Node::SharedPtr ROS node
   */
  virtual void initialize(rclcpp::Node::SharedPtr node) override
  {
    // initialize base class
    initialize_provider_base(node, "ChatOllamaProvider", "");
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
  virtual Poco::JSON::Object toJson(const prompt::PromptRequest& prompt)
  {
    // add options
    Poco::JSON::Object result = handle_options(prompt);

    prompt::PromptDialogue dialog_;
    dialog_.role = "user";
    dialog_.content = prompt.prompt;

    conversation_.push_back(dialog_);

    Poco::JSON::Array messages_array;

    for (const prompt::PromptDialogue& dialog_ : conversation_)
    {
      Poco::JSON::Object dialog_object_;

      dialog_object_.set("role", dialog_.role);
      dialog_object_.set("content", dialog_.content);

      messages_array.add(dialog_object_);
    }

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
  virtual prompt::PromptResponse fromJson(const Poco::JSON::Object::Ptr object)
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

    prompt::PromptDialogue dialog_;
    dialog_.role = "assistant";
    dialog_.content = res.response;

    conversation_.push_back(dialog_);

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
