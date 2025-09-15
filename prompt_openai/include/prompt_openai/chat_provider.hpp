#pragma once

#include <prompt_base/prompt_base_class.hpp>

namespace prompt
{

/**
 * @brief ChatOpenAIProvider
 *
 * This is a prompt provider that uses a REST API to send and receive prompts
 * the typical rest api uses application/json content type so that is what is
 * supported
 *
 */
class ChatOpenAIProvider : public PromptBaseClass
{
public:
  /**
   * @brief Construct a new Chat Prompt Provider OpenAI object
   *
   */
  ChatOpenAIProvider() : PromptBaseClass()
  {
  }

  /**
   * @brief Initialize the ChatOpenAIProvider
   *
   * This method initializes the ChatOpenAIProvider with parameters from the ROS parameter server.
   *
   * @param node rclcpp::Node::SharedPtr ROS node
   */
  virtual void initialize(rclcpp::Node::SharedPtr node) override
  {
    // initialize base class
    initialize_provider_base(node, "ChatOpenAIProvider", "OPENAI_API_KEY");
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
    std::ostringstream jsonStream;
    object->stringify(jsonStream);

    // TODO: create custom parsers for specific options from different apis
    // res.success = object->get("success").convert<bool>();
    // res.accuracy = object->get("accuracy").convert<double>();
    // res.confidence = object->get("confidence").convert<double>();
    // res.risk = object->get("risk").convert<double>();
    // for all other variables loop and push back to response key/values

    prompt::PromptResponse res;

    // Ensure "choices" exists
    if (object->has("choices"))
    {
      Poco::JSON::Array::Ptr choicesArray = object->getArray("choices");

      // Check if the array is not empty
      if (choicesArray->size() > 0)
      {
        // Extract the first object from the array
        Poco::JSON::Object::Ptr choiceObj = choicesArray->getObject(0);

        // Ensure "message" exists
        if (choiceObj->has("message"))
        {
          Poco::JSON::Object::Ptr messageObj = choiceObj->getObject("message");

          // Ensure "content" exists
          if (messageObj->has("content"))
          {
            res.response = messageObj->getValue<std::string>("content");
          }
        }
      }
    }

    for (Poco::JSON::Object::ConstIterator it = object->begin(); it != object->end(); ++it)
    {
      if ((it->first != "choices") && (it->first != "usage"))
      {
        try
        {
          if (!it->second.isEmpty() && it->second.isString())
          {
            res.options.push_back(prompt::PromptOption{ it->first, it->second.convert<std::string>(), "" });
          }
          else if (!it->second.isEmpty())
          {
            res.options.push_back(prompt::PromptOption{ it->first, it->second.toString(), "" });
          }
          else
          {
            res.options.push_back(prompt::PromptOption{ it->first, "[null]", "" });
          }
        }
        catch (const Poco::Exception& ex)
        {
          RCLCPP_WARN(node_->get_logger(), "Failed to convert JSON key '%s': %s", it->first.c_str(), ex.what());
        }
      }
    }

    size_t startPos = res.response.find("```xml\n");
    if (startPos != std::string::npos)
    {
      res.response.replace(startPos, 6, "");  // Remove "```xml\n"
    }

    size_t endPos = res.response.rfind("\n```");
    if (endPos != std::string::npos)
    {
      res.response.replace(endPos, 4, "");  // Remove "\n```"
    }

    prompt::PromptDialogue dialog_;
    dialog_.role = "assistant";
    dialog_.content = res.response;

    conversation_.push_back(dialog_);

    return res;
  }

private:
  std::vector<prompt::PromptDialogue> conversation_;
};

}  // namespace prompt
