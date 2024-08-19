#pragma once

#include <llm_prompt_provider_plugins/prompt_provider_base.hpp>

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

/**
 * @brief RestPromptProvider
 *
 * This is a prompt provider that uses a REST API to send and receive prompts
 * the typical rest api uses application/json content type so that is what is
 * supported
 *
 */
class RestPromptProvider : public PromptProviderBase
{
public:
  // constructor
  RestPromptProvider() : uri_("")
  {
  }
  virtual void init(rclcpp::node_interfaces::NodeParametersInterface::SharedPtr params,
                    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr log);
  virtual const PromptResponse sendPrompt(const PromptRequest& req);

  static const Poco::JSON::Object toJson(const PromptRequest& prompt)
  {
    Poco::JSON::Object result;
    result.set("prompt", prompt.prompt);

    // options
    for (const auto& option : prompt.options)
    {
      result.set(option.key, option.value);
    }

    return result;
  }

protected:
  virtual void handle_opts(const PromptRequest& req);

private:
  std::string uri_;
  std::string method_;
};

}  // namespace prompt_provider
