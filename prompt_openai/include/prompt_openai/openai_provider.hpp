#pragma once

#include <prompt_base/prompt_base_class.hpp>

namespace prompt
{

/**
 * @brief OpenAIProvider
 *
 * This is a prompt provider that uses a REST API to send and receive prompts
 * the typical rest api uses application/json content type so that is what is
 * supported
 *
 */
class OpenAIProvider : public PromptBaseClass
{
public:
  /**
   * @brief Construct a new Chat Prompt Provider OpenAI object
   *
   */
  OpenAIProvider() : PromptBaseClass()
  {
  }

  /**
   * @brief Initialize the OpenAIProvider
   *
   * This method initializes the OpenAIProvider with parameters from the ROS parameter server.
   *
   * @param node rclcpp::Node::SharedPtr ROS node
   */
  virtual void initialize(rclcpp::Node::SharedPtr node) override
  {
    // initialize base class
    initialize_prompt_base(node, "OpenAIProvider", "OPENAI_API_KEY");
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
   // add options (model, temperature, etc.)
   Poco::JSON::Object result = handle_options(prompt.options);

    // /v1/responses uses `input` instead of `prompt`
    // For simple one-shot prompts, send the text directly as a string.
    result.set("input", prompt.prompt);

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
                                                std::vector<PromptDialogue>& conversation) override
  {
    // add options (model, temperature, etc.)
    Poco::JSON::Object result = handle_options(prompt.options);

    // /v1/responses represents conversational input as an array of "message" items
    // Each message has: role + content array with input_text/output_text entries.
    Poco::JSON::Array input_array;

    // add previous conversation
    for (const prompt::PromptDialogue& dialog : conversation)
    {
      Poco::JSON::Object message_obj;
      message_obj.set("type", "message");
      message_obj.set("role", dialog.role);

      Poco::JSON::Array content_array;
      Poco::JSON::Object content_obj;
      content_obj.set("type", "input_text");
      content_obj.set("text", dialog.content);
      content_array.add(content_obj);

      message_obj.set("content", content_array);
      input_array.add(message_obj);
    }

    // add latest user prompt as the final message
    Poco::JSON::Object latest_message;
    latest_message.set("type", "message");
    latest_message.set("role", "user");

    Poco::JSON::Array latest_content_array;
    Poco::JSON::Object latest_content_obj;
    latest_content_obj.set("type", "input_text");
    latest_content_obj.set("text", prompt.prompt);
    latest_content_array.add(latest_content_obj);

    latest_message.set("content", latest_content_array);
    input_array.add(latest_message);

    // attach to body as `input`
    result.set("input", input_array);

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

    // Extract assistant text from /v1/responses `output` structure
    if (object->has("output"))
    {
      try
      {
        Poco::JSON::Array::Ptr output_array = object->getArray("output");
        for (size_t i = 0; i < output_array->size(); ++i)
        {
          Poco::JSON::Object::Ptr out_item = output_array->getObject(i);
          if (!out_item->has("content"))
            continue;

          Poco::JSON::Array::Ptr content_array = out_item->getArray("content");
          for (size_t j = 0; j < content_array->size(); ++j)
          {
            Poco::JSON::Object::Ptr content_obj = content_array->getObject(j);
            if (content_obj->has("text"))
            {
              res.response = content_obj->getValue<std::string>("text");
              break;
            }
          }

          if (!res.response.empty())
            break;
        }
      }
      catch (const Poco::Exception& ex)
      {
        RCLCPP_WARN(node_->get_logger(), "Failed to parse OpenAI /v1/responses output: %s", ex.what());
      }
    }

    res.success = !res.response.empty();

    // attach remaining top-level fields as options (excluding large nested structures)
    for (Poco::JSON::Object::ConstIterator it = object->begin(); it != object->end(); ++it)
    {
      if ((it->first == "output") || (it->first == "usage"))
        continue;

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
  virtual prompt::PromptResponse fromJsonConversation(const Poco::JSON::Object::Ptr object) override
  {
    prompt::PromptResponse res;
    // Parse the same /v1/responses structure for conversational responses
    if (object->has("output"))
    {
      try
      {
        Poco::JSON::Array::Ptr output_array = object->getArray("output");
        for (size_t i = 0; i < output_array->size(); ++i)
        {
          Poco::JSON::Object::Ptr out_item = output_array->getObject(i);
          if (!out_item->has("content"))
            continue;

          Poco::JSON::Array::Ptr content_array = out_item->getArray("content");
          for (size_t j = 0; j < content_array->size(); ++j)
          {
            Poco::JSON::Object::Ptr content_obj = content_array->getObject(j);
            if (content_obj->has("text"))
            {
              res.response = content_obj->getValue<std::string>("text");
              break;
            }
          }

          if (!res.response.empty())
            break;
        }
      }
      catch (const Poco::Exception& ex)
      {
        RCLCPP_WARN(node_->get_logger(), "Failed to parse OpenAI /v1/responses output (conversation): %s", ex.what());
      }
    }

    res.success = !res.response.empty();

    // Attach remaining top-level fields as options (excluding large nested structures)
    for (Poco::JSON::Object::ConstIterator it = object->begin(); it != object->end(); ++it)
    {
      if ((it->first == "output") || (it->first == "usage"))
        continue;

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

    // Clean up any fenced XML blocks if present (for backward compatibility)
    size_t startPos = res.response.find("```xml\n");
    if (startPos != std::string::npos)
    {
      res.response.replace(startPos, 6, "");
    }

    size_t endPos = res.response.rfind("\n```");
    if (endPos != std::string::npos)
    {
      res.response.replace(endPos, 4, "");
    }

    return res;
  }
};

}  // namespace prompt
