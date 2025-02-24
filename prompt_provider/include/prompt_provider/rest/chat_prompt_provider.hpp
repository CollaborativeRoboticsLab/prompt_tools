#pragma once
#include <prompt_msgs/msg/prompt.hpp>
#include <prompt_provider/prompt_provider_base.hpp>
#include <prompt_utils/structs.hpp>
#include <prompt_utils/exceptions.hpp>
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
 * @brief ChatPromptProvider
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
class ChatPromptProvider : public prompt_provider::PromptProviderBase
{
public:
  // constructor
  ChatPromptProvider() : uri_("")
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
    uri_ = params->declare_parameter("rest.ChatPromptProvider.uri", rclcpp::ParameterValue(default_uri))
               .get<std::string>();

    // get method from parameter server
    method_ =
        params->declare_parameter("rest.ChatPromptProvider.method", rclcpp::ParameterValue("POST")).get<std::string>();

    // get verification mode from parameter server
    ssl_verify_ =
        params->declare_parameter("rest.ChatPromptProvider.ssl_verify", rclcpp::ParameterValue(true)).get<bool>();

    // get auth type from parameter server
    auth_type_ = params->declare_parameter("rest.ChatPromptProvider.auth_type", rclcpp::ParameterValue("Bearer"))
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
    RCLCPP_INFO(logging_->get_logger(), "ChatPromptProvider initialized with uri: %s, method: %s", uri_.c_str(),
                method_.c_str());
  }

  /**
   * @brief sendPrompt send a prompt to a prompt provider using REST
   *
   * Typical providers offer stream based responses which is also supported
   *
   * @param req
   * @return const PromptResponse
   */
  virtual const prompt::PromptResponse sendPrompt(const prompt::PromptRequest& req)
  {
    // uri
    Poco::URI uri(uri_);

    // prepare request body
    Poco::JSON::Object body_json = ChatPromptProvider::toJson(req);

    // calculate body length
    std::ostringstream body_stream;
    body_json.stringify(body_stream);

    // create request object
    Poco::Net::HTTPRequest request(method_, uri.getPath());
    // set headers
    request.setContentType("application/json");
    request.setContentLength(body_stream.str().size());

    // if bearer token
    if (auth_type_ == "Bearer")
    {
      request.setCredentials(auth_type_, api_key_);
    }
    // else if (auth_type_ == "Token")
    // {
    //   request.setCredentials(auth_type_, api_key_);
    // }
    else
    {
      RCLCPP_WARN(logging_->get_logger(), "unsupported auth type: %s", auth_type_.c_str());
    }

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

      RCLCPP_DEBUG(logging_->get_logger(), "secure session created");
    }
    else
    {
      // create insecure session
      // Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
      session_ptr = std::make_unique<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());
      RCLCPP_WARN(logging_->get_logger(), "insecure session created");
    }

    try
    {
      // send request
      std::ostream& os = session_ptr->sendRequest(request);
      // complete request body
      body_json.stringify(os);

      // RCLCPP_WARN(logging_->get_logger(), "sending prompt: %s", body_stream.str().c_str());
    }
    catch (const Poco::Net::NetException& e)
    {
      RCLCPP_ERROR(logging_->get_logger(), "network error: %s", e.what());
      throw prompt::PromptProviderException("network error: " + std::string(e.what()));
    }

    // get response
    Poco::Net::HTTPResponse response;
    std::istream& rs = session_ptr->receiveResponse(response);

    // check for errors
    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK)
    {
      RCLCPP_ERROR(logging_->get_logger(), "HTTP Error: %i, %s", response.getStatus(), response.getReason().c_str());
      throw prompt::PromptProviderException("HTTP Error: " + std::to_string(response.getStatus()) + " " + response.getReason());
    }

    // check content type is 'text/event-stream' or 'application/x-ndjson'
    // https://developer.mozilla.org/en-US/docs/Web/API/Server-sent_events
    // these types are used for server-sent events or newline delimited JSON
    if (response.getContentType() == "text/event-stream" || response.getContentType() == "application/x-ndjson")
    {
      // TODO: handle event stream
      // pass to stream parsing
      // ChatPromptProvider::handle_event_stream(rs, chunck_cb);
      RCLCPP_ERROR(logging_->get_logger(), "HTTP streaming not supported");
      throw prompt::PromptProviderException("HTTP stream not supported");
    }

    // is the response chunked even though it is not server-sent event?
    if (response.getChunkedTransferEncoding())
    {
      // TODO: handle chunked responses
      RCLCPP_ERROR(logging_->get_logger(), "HTTP Chunked Transfer Encoding not supported");
      throw prompt::PromptProviderException("HTTP Chunked Transfer Encoding not supported");
    }

    // parse response
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(rs);
    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

    // create prompt provider response container
    prompt::PromptResponse res = handle_response(object);

    return res;
  }

  virtual const Poco::JSON::Object toJson(const prompt::PromptRequest& prompt)
  {
    // add options
    Poco::JSON::Object result = handle_options(prompt);

    prompt::PromptDialogue dialog_;
    dialog_.role = "user";
    dialog_.content = prompt.prompt;

    conversation_.push_back(dialog_);

    Poco::JSON::Array messages_array = handle_conversation();

    // add prompt
    result.set("messages", messages_array);

    return result;
  }

protected:
  virtual const prompt::PromptResponse handle_response(const Poco::JSON::Object::Ptr object)
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

  virtual const Poco::JSON::Array handle_conversation()
  {
    // flatten conversation into a object
    Poco::JSON::Array messages_;

    for (const prompt::PromptDialogue& dialog_ : conversation_)
    {
      Poco::JSON::Object dialog_object_;

      dialog_object_.set("role", dialog_.role);
      dialog_object_.set("content", dialog_.content);

      messages_.add(dialog_object_);
    }

    return messages_;
  }

  virtual const Poco::JSON::Object handle_options(const prompt::PromptRequest& prompt)
  {
    // flatten options into object
    Poco::JSON::Object result;

    for (const prompt::PromptOption& option : prompt.options)
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
  std::string auth_type_;
  std::string api_key_;

  std::vector<prompt::PromptDialogue> conversation_;
};

}  // namespace rest

}  // namespace prompt_provider
