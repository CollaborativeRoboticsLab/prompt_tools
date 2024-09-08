#pragma once

#include <llm_prompt_provider_plugins/prompt_provider_base.hpp>
#include <prompt_msgs/msg/prompt.hpp>

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
 * Another typical feature of these REST APIs is the use of OpenAI's API definition
 * This definition includes:
 *  - authentication using a bearer token
 *  - /completions endpoint
 *  - /chat/completions endpoint
 *  - /edits endpoint
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
                    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr log)
  {
    // set logger
    node_logging_interface_ptr_ = log;

    // default uri
    std::string default_uri = "https://localhost:8443/api/v1/prompt";

    // get uri from parameter server
    uri_ = params->declare_parameter("RestPromptProvider.uri", rclcpp::ParameterValue(default_uri)).get<std::string>();

    // get method from parameter server
    method_ = params->declare_parameter("RestPromptProvider.method", rclcpp::ParameterValue("POST")).get<std::string>();

    // get verification mode from parameter server
    ssl_verify_ = params->declare_parameter("RestPromptProvider.ssl_verify", rclcpp::ParameterValue(true)).get<bool>();

    // log
    RCLCPP_INFO(node_logging_interface_ptr_->get_logger(), "RestPromptProvider initialized with uri: %s, method: %s",
                uri_.c_str(), method_.c_str());
  }

  virtual const PromptResponse sendPrompt(const PromptRequest& req)
  {
    // uri
    Poco::URI uri(uri_);

    // prepare request body
    Poco::JSON::Object body_json = RestPromptProvider::toJson(req);

    // calculate body length
    std::ostringstream body_stream;
    body_json.stringify(body_stream);

    // create request object
    Poco::Net::HTTPRequest request(method_, uri.getPath());
    // set headers
    request.setContentType("application/json");
    request.setContentLength(body_stream.str().size());

    std::unique_ptr<Poco::Net::HTTPClientSession> session_ptr;
    // is the session secure?
    if (uri.getScheme() == "https")
    {
      // context without certificate verification
      Poco::Net::Context::Params params;

      if (!ssl_verify_)
      {
        params.verificationMode = Poco::Net::Context::VERIFY_NONE;
      }

      Poco::Net::Context::Ptr context = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, params);

      // create secure session
      // Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort(), context);
      session_ptr = std::make_unique<Poco::Net::HTTPSClientSession>(uri.getHost(), uri.getPort(), context);
      RCLCPP_DEBUG(node_logging_interface_ptr_->get_logger(), "secure session created");
    }
    else
    {
      // create insecure session
      // Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
      session_ptr = std::make_unique<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());
      RCLCPP_WARN(node_logging_interface_ptr_->get_logger(), "insecure session created");
    }

    // send request
    std::ostream& os = session_ptr->sendRequest(request);

    RCLCPP_WARN(node_logging_interface_ptr_->get_logger(), "Sending prompt: %s", body_stream.str().c_str());

    // complete request body
    body_json.stringify(os);

    // get response
    Poco::Net::HTTPResponse response;
    std::istream& rs = session_ptr->receiveResponse(response);

    // parse response
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(rs);
    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

    // check for errors
    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK)
    {
      RCLCPP_ERROR(node_logging_interface_ptr_->get_logger(), "HTTP Error: %i, %s", response.getStatus(),
                   response.getReason().c_str());
      throw PromptProviderException("HTTP Error: " + std::to_string(response.getStatus()) + " " + response.getReason());
    }

    // create response
    PromptProviderBase::PromptResponse res;

    // try parse response
    if (object->get("response"))
      res.response = object->get("response").toString();

    // for all other variables loop and push back to response key/values
    for (auto it = object->begin(); it != object->end(); ++it)
    {
      // FIXME:
      // res.options.push_back(PromptProviderBase::PromptOption{ it->key(), it->value(), "" });
    }

    // res.success = object->get("success").convert<bool>();
    // res.accuracy = object->get("accuracy").convert<double>();
    // res.confidence = object->get("confidence").convert<double>();
    // res.risk = object->get("risk").convert<double>();

    return res;
  }

  virtual const Poco::JSON::Object toJson(const PromptRequest& prompt)
  {
    // add options
    Poco::JSON::Object result = handle_opts(prompt);

    // add prompt
    result.set("prompt", prompt.prompt);

    return result;
  }

protected:
  virtual const Poco::JSON::Object handle_opts(const PromptRequest& prompt)
  {
    // flatten options into object
    Poco::JSON::Object result;
    for (const PromptProviderBase::PromptOption& option : prompt.options)
    {
      // try cast the value if there is a type hint
      if (option.type == prompt_msgs::msg::ModelOption::STRING_TYPE)
      {
        result.set(option.key, option.value);
        continue;
      }

      // try cast the value if there is a type hint
      if (option.type == prompt_msgs::msg::ModelOption::BOOL_TYPE)
      {
        result.set(option.key, (option.value == "true") ? true : false);
        continue;
      }

      // try cast the value if there is a type hint
      if (option.type == prompt_msgs::msg::ModelOption::INT_TYPE)
      {
        result.set(option.key, std::stoi(option.value));
        continue;
      }

      // try cast the value if there is a type hint
      if (option.type == prompt_msgs::msg::ModelOption::REAL_TYPE)
      {
        result.set(option.key, std::stod(option.value));
        continue;
      }

      // just set the value if there is no type hint
      result.set(option.key, option.value);
    }

    return result;
  }

private:
  std::string uri_;
  std::string method_;
  bool ssl_verify_;
};

}  // namespace prompt_provider
