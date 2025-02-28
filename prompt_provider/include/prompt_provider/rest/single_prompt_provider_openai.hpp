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
 * @brief SinglePromptProviderOpenAI
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
class SinglePromptProviderOpenAI : public prompt_provider::rest::RestProviderBase
{
public:
  // constructor
  SinglePromptProviderOpenAI() : RestProviderBase()
  {
  }

  virtual void init(rclcpp::node_interfaces::NodeParametersInterface::SharedPtr params,
                    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr log)
  {
    // set logger
    logging_ = log;

    // default uri
    std::string default_uri = "https://localhost:8443/api/v1/prompt";

    // get uri from parameter server
    uri_ = params->declare_parameter("rest.SinglePromptProviderOpenAI.uri", rclcpp::ParameterValue(default_uri))
               .get<std::string>();

    // get method from parameter server
    method_ = params->declare_parameter("rest.SinglePromptProviderOpenAI.method", rclcpp::ParameterValue("POST"))
                  .get<std::string>();

    // get verification mode from parameter server
    ssl_verify_ =
        params->declare_parameter("rest.SinglePromptProviderOpenAI.ssl_verify", rclcpp::ParameterValue(true)).get<bool>();

    // get auth type from parameter server
    auth_type_ = params->declare_parameter("rest.SinglePromptProviderOpenAI.auth_type", rclcpp::ParameterValue("Bearer"))
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
    RCLCPP_INFO(logging_->get_logger(), "SinglePromptProviderOpenAI initialized with uri: %s, method: %s", uri_.c_str(),
                method_.c_str());
  }

protected:
  virtual const Poco::JSON::Object toJson(const prompt::PromptRequest& prompt) override
  {
    // add options
    Poco::JSON::Object result = handle_options(prompt);

    // add prompt
    result.set("prompt", prompt.prompt);

    return result;
  }

  virtual const prompt::PromptResponse fromJson(const Poco::JSON::Object::Ptr object) override
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

}  // namespace rest

}  // namespace prompt_provider
