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
  RestPromptProvider() : uri_("") {};

  virtual void init(const rclcpp::Node::SharedPtr& node)
  {
    // get uri from parameter server
    uri_ = node->declare_parameter<std::string>("uri", "https://localhost:8443/api/v1/"
                                                       "prompt");
    method_ = node->declare_parameter<std::string>("method", "POST");
  }

  virtual const PromptResponse sendPrompt(const PromptRequest& req)
  {
    // uri
    Poco::URI uri(uri_);

    // context without certificate verification
    Poco::Net::Context::Params params;
    params.verificationMode = Poco::Net::Context::VERIFY_NONE;
    Poco::Net::Context::Ptr context =
        new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, params);

    // create https session
    Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort(), context);

    // prepare request body
    Poco::JSON::Object body_json = RestPromptProvider::toJson(req);

    // calculate body length
    std::ostringstream body_stream;
    body_json.stringify(body_stream);

    // create request object
    Poco::Net::HTTPRequest request(method_, uri.getPath());
    // set headers
    request.setContentType("application/json");

    // send request
    std::ostream& os = session.sendRequest(request);

    // complete request body
    body_json.stringify(os);

    // get response
    Poco::Net::HTTPResponse response;
    std::istream& rs = session.receiveResponse(response);

    // parse response
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(rs);
    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

    // check for errors
    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK)
    {
      throw PromptProviderException("HTTP Error: " + response.getReason());
    }

    // create response
    PromptResponse res;
    res.response = object->get("response").toString();
    res.success = object->get("success").convert<bool>();
    res.accuracy = object->get("accuracy").convert<double>();
    res.confidence = object->get("confidence").convert<double>();
    res.risk = object->get("risk").convert<double>();

    return res;
  }

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
