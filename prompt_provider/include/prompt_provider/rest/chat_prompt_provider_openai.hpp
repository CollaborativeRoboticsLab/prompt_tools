#pragma once
#include <prompt_msgs/msg/prompt.hpp>
#include <prompt_provider/rest/rest_provider_base.hpp>
#include <prompt_utils/exceptions.hpp>
#include <prompt_utils/structs.hpp>
#include <rclcpp/rclcpp.hpp>

// include poco json and net/netssl
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/NetException.h>
#include <Poco/URI.h>

namespace prompt_provider
{
namespace rest
{

/**
 * @brief ChatPromptProviderOpenAI
 *
 * This is a prompt provider that uses a REST API to send and receive prompts
 * the typical rest api uses application/json content type so that is what is
 * supported
 *
 * Another typical feature of these REST APIs is the use of OpenAI's API definition
 * This definition includes:
 *  - authentication using a bearer token
 *  - /completions endpoint
 *  - /chat/completions endpoint
 *  - /edits endpoint
 *
 */
class ChatPromptProviderOpenAI : public prompt_provider::rest::RestProviderBase
{
public:
  // constructor
  ChatPromptProviderOpenAI() : RestProviderBase()
  {
  }

  virtual void init(rclcpp::node_interfaces::NodeParametersInterface::SharedPtr params,
                    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr log)
  {
    // set logger
    logging_ = log;

    // default uri
    std::string default_uri = "https://localhost:8443/api/v1/chat";

    // get uri from parameter server
    uri_ = params->declare_parameter("rest.ChatPromptProviderOpenAI.uri", rclcpp::ParameterValue(default_uri))
               .get<std::string>();

    // get method from parameter server
    method_ = params->declare_parameter("rest.ChatPromptProviderOpenAI.method", rclcpp::ParameterValue("POST"))
                  .get<std::string>();

    // get verification mode from parameter server
    ssl_verify_ =
        params->declare_parameter("rest.ChatPromptProviderOpenAI.ssl_verify", rclcpp::ParameterValue(true)).get<bool>();

    // get auth type from parameter server
    auth_type_ = params->declare_parameter("rest.ChatPromptProviderOpenAI.auth_type", rclcpp::ParameterValue("Bearer"))
                     .get<std::string>();

    // get api key from environment
    const char* api_key_env = std::getenv("PROMPT_PROVIDER_API_KEY");
    if (api_key_env)
    {
      api_key_ = api_key_env;
    }
    else
    {
      RCLCPP_WARN(logging_->get_logger(), "missing env var: PROMPT_PROVIDER_API_KEY");
      api_key_ = "";
    }

    // log
    RCLCPP_INFO(logging_->get_logger(), "ChatPromptProviderOpenAI initialized with uri: %s, method: %s", uri_.c_str(),
                method_.c_str());
  }

protected:
  virtual const Poco::JSON::Object toJson(const prompt::PromptRequest& prompt)
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

  virtual const prompt::PromptResponse fromJson(const Poco::JSON::Object::Ptr object)
  {
    std::ostringstream jsonStream;
    object->stringify(jsonStream);
    RCLCPP_INFO(logging_->get_logger(), "Poco JSON Object: %s", jsonStream.str().c_str());

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
        res.options.push_back(prompt::PromptOption{ it->first, it->second.convert<std::string>(), "" });
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

}  // namespace rest

}  // namespace prompt_provider
