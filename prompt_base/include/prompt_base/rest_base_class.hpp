#pragma once

#include <string>

// include poco json and net/netssl
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/NetException.h>
#include <Poco/URI.h>

#include <prompt_base/base_class.hpp>
#include <prompt_base/utils/file_part_source.hpp>
#include <prompt_msgs/msg/prompt.hpp>

namespace prompt
{
/**
 * @brief RestBaseClass
 *
 * This class implements a base class for RESTful prompt plugins.
 * It provides a common interface for sending prompts and receiving responses
 *
 */
class RestBaseClass : public BaseClass
{
public:
  RestBaseClass() : uri_("")
  {
  }

  virtual ~RestBaseClass() = default;

  /**
   * @brief Initialize the REST base class
   *
   * This method initializes the REST base class with parameters from the ROS parameter server.
   *
   * @param node rclcpp::Node::SharedPtr ROS node
   * @param plugin_name std::string Name of the plugin, used for parameter names
   * @param api_key_name std::string Name of the API key parameter, if different from the default
   * @throws prompt::PromptException if there is an error during initialization
   */
  virtual void initialize_rest_base(rclcpp::Node::SharedPtr node, std::string plugin_name = "RestBaseClass",
                                    std::string api_key_name = "")
  {
    // initialize base class
    initialize_base(node, plugin_name);

    // declare parameters
    node_->declare_parameter(plugin_name_ + ".rest.uri", "http://localhost:8000/api/v1/prompt");
    node_->declare_parameter(plugin_name_ + ".rest.method", "POST");
    node_->declare_parameter(plugin_name_ + ".rest.ssl_verify", true);
    node_->declare_parameter(plugin_name_ + ".rest.auth_type", "Bearer");

    // get parameters from the parameter server
    uri_ = node_->get_parameter(plugin_name_ + ".rest.uri").as_string();
    method_ = node_->get_parameter(plugin_name_ + ".rest.method").as_string();
    ssl_verify_ = node_->get_parameter(plugin_name_ + ".rest.ssl_verify").as_bool();
    auth_type_ = node_->get_parameter(plugin_name_ + ".rest.auth_type").as_string();

    // get api key from environment
    if (!api_key_name.empty())
    {
      const char* api_key_env = std::getenv(api_key_name.c_str());
      if (api_key_env)
      {
        api_key_ = api_key_env;
        RCLCPP_INFO(node_->get_logger(), "API key loaded from environment variables");
      }
      else
      {
        RCLCPP_WARN(node_->get_logger(), "missing env variable: %s", api_key_name.c_str());
        throw prompt::PromptException("missing env variable: " + api_key_name);
        api_key_ = "";
      }
    }
    {
      RCLCPP_INFO(node_->get_logger(), "An API key is not used for this plugin, using empty string");
      api_key_ = "";
    }

    // log the parameters
    RCLCPP_INFO(node_->get_logger(), "%s URI: %s", plugin_name_.c_str(), uri_.c_str());
    RCLCPP_INFO(node_->get_logger(), "%s Method: %s", plugin_name_.c_str(), method_.c_str());
    RCLCPP_INFO(node_->get_logger(), "%s SSL Verify: %s", plugin_name_.c_str(), ssl_verify_ ? "true" : "false");
    RCLCPP_INFO(node_->get_logger(), "%s Auth Type: %s", plugin_name_.c_str(), auth_type_.c_str());
    RCLCPP_INFO(node_->get_logger(), "%s Plugin initialized", plugin_name_.c_str());
  }

  /**
   * @brief sendPrompt send a prompt to a prompt provider using REST
   *
   * Typical providers offer stream based responses which is also supported
   *
   * @param req The prompt request containing the prompt and options.
   * @return const PromptResponse
   */
  virtual prompt::PromptResponse sendPrompt(const prompt::PromptRequest& req)
  {
    // uri
    Poco::URI uri(uri_);

    // prepare request body
    Poco::JSON::Object body_json = toJson(req);

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

    // Todo: support other auth types
    // else if (auth_type_ == "Token")
    // {
    //   request.setCredentials(auth_type_, api_key_);
    // }
    else
    {
      RCLCPP_WARN(node_->get_logger(), "unsupported auth type: %s", auth_type_.c_str());
    }

    // RCLCPP_INFO(node_->get_logger(), "Port %d", uri.getPort());

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
      else
      {
        params.verificationMode = Poco::Net::Context::VERIFY_STRICT;
        params.caLocation = "/etc/ssl/certs";  // Update this path if needed
      }

      Poco::Net::Context::Ptr context = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, params);

      // create secure session
      // Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort(), context);
      session_ptr = std::make_unique<Poco::Net::HTTPSClientSession>(uri.getHost(), uri.getPort(), context);
      RCLCPP_DEBUG(node_->get_logger(), "secure session created");
    }
    else
    {
      // create insecure session
      // Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
      session_ptr = std::make_unique<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());
      RCLCPP_WARN(node_->get_logger(), "insecure session created");
    }

    try
    {
      // send request
      std::ostream& os = session_ptr->sendRequest(request);
      // complete request body
      body_json.stringify(os);
    }
    catch (const Poco::Net::NetException& e)
    {
      RCLCPP_ERROR(node_->get_logger(), "network error: %s", e.what());
      throw prompt::PromptException("network error: " + std::string(e.what()));
    }

    // get response
    Poco::Net::HTTPResponse response;
    std::istream& rs = session_ptr->receiveResponse(response);

    // check for errors
    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK)
    {
      RCLCPP_ERROR(node_->get_logger(), "HTTP Error: %i, %s", response.getStatus(), response.getReason().c_str());
      throw prompt::PromptException("HTTP Error: " + std::to_string(response.getStatus()) + " " + response.getReason());
    }

    // check content type is 'text/event-stream' or 'application/x-ndjson'
    // https://developer.mozilla.org/en-US/docs/Web/API/Server-sent_events
    // these types are used for server-sent events or newline delimited JSON
    if (response.getContentType() == "text/event-stream" || response.getContentType() == "application/x-ndjson")
    {
      // TODO: handle event stream
      // pass to stream parsing
      // SinglePromptProvider::handle_event_stream(rs, chunck_cb);
      RCLCPP_ERROR(node_->get_logger(), "HTTP streaming not supported");
      throw prompt::PromptException("HTTP stream not supported");
    }

    // is the response chunked even though it is not server-sent event?
    if (response.getChunkedTransferEncoding())
    {
      // TODO: handle chunked responses
      RCLCPP_ERROR(node_->get_logger(), "HTTP Chunked Transfer Encoding not supported");
      throw prompt::PromptException("HTTP Chunked Transfer Encoding not supported");
    }

    // parse response
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(rs);
    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

    // create prompt provider response container
    prompt::PromptResponse res = fromJson(object);

    return res;
  }

  /**
   * @brief sendPrompt send a prompt to a prompt provider using REST
   *
   * Typical providers offer stream based responses which is also supported
   *
   * @param req
   * @return const PromptResponse
   */
  virtual prompt::PromptResponse sendPromptAudio(const prompt::PromptRequest& req)
  {
    Poco::URI uri(uri_);

    // Create HTTP(S) session
    std::unique_ptr<Poco::Net::HTTPClientSession> session_ptr;

    if (uri.getScheme() == "https")
    {
      Poco::Net::Context::Params params;
      params.verificationMode = ssl_verify_ ? Poco::Net::Context::VERIFY_STRICT : Poco::Net::Context::VERIFY_NONE;
      params.caLocation = "/etc/ssl/certs";

      Poco::Net::Context::Ptr context = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, params);
      session_ptr = std::make_unique<Poco::Net::HTTPSClientSession>(uri.getHost(), uri.getPort(), context);
      RCLCPP_DEBUG(node_->get_logger(), "Secure session created");
    }
    else
    {
      session_ptr = std::make_unique<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());
      RCLCPP_WARN(node_->get_logger(), "Insecure session created");
    }

    // Prepare request
    Poco::Net::HTTPRequest request(method_, uri.getPathAndQuery(), Poco::Net::HTTPMessage::HTTP_1_1);

    if (auth_type_ == "Bearer")
      request.set("Authorization", "Bearer " + api_key_);
    else
      RCLCPP_WARN(node_->get_logger(), "Unsupported auth type: %s", auth_type_.c_str());

    try
    {
      std::ostream& os = session_ptr->sendRequest(request);

      if (req.contains_audio && !req.audio_buffer.empty())
      {
        // Use multipart form for audio buffer
        Poco::Net::HTMLForm form;
        form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);

        // check for model option
        bool is_model_set = false;
        bool is_response_format_set = false;

        for (const auto& opt : req.options)
        {
          if (opt.key != "is_memory_audio" && opt.key != "file_type")
            form.set(opt.key, opt.value);

          if (opt.key == "model")
            is_model_set = true;

          if (opt.key == "response_format")
            is_response_format_set = true;
        }

        if (!is_model_set)
        {
          RCLCPP_WARN(node_->get_logger(), "Model option not set, using default model");
          form.set("model", "gpt-4o-transcribe");
        }

        if (!is_response_format_set)
        {
          RCLCPP_WARN(node_->get_logger(), "Response format option not set, using default response format");
          form.set("response_format", "json");
        }

        if (req.prompt.empty())
        {
          RCLCPP_WARN(node_->get_logger(), "Prompt is empty, using default prompt");
          form.set("prompt", "Transcribe the audio");
        }
        else
        {
          form.set("prompt", req.prompt);
        }

        form.addPart("file", new FilePartSource(req.file_type, req.audio_buffer));
        form.prepareSubmit(request);
        form.write(os);
      }
      else
      {
        // Fall back to JSON
        Poco::JSON::Object body_json = toJson(req);
        std::ostringstream body_stream;
        body_json.stringify(body_stream);

        request.setContentType("application/json");
        request.setContentLength(body_stream.str().size());
        os << body_stream.str();
      }
    }
    catch (const Poco::Exception& e)
    {
      RCLCPP_ERROR(node_->get_logger(), "Request error: %s", e.what());
      throw prompt::PromptException("Request error: " + std::string(e.what()));
    }

    // Receive response
    Poco::Net::HTTPResponse response;
    std::istream& rs = session_ptr->receiveResponse(response);

    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK)
    {
      RCLCPP_ERROR(node_->get_logger(), "HTTP Error: %i, %s", response.getStatus(), response.getReason().c_str());
      throw prompt::PromptException("HTTP Error: " + std::to_string(response.getStatus()) + " " + response.getReason());
    }

    if (response.getContentType() == "text/event-stream" || response.getContentType() == "application/x-ndjson")
    {
      RCLCPP_ERROR(node_->get_logger(), "HTTP streaming not supported");
      throw prompt::PromptException("HTTP stream not supported");
    }

    if (response.getChunkedTransferEncoding())
    {
      RCLCPP_ERROR(node_->get_logger(), "HTTP Chunked Transfer Encoding not supported");
      throw prompt::PromptException("HTTP Chunked Transfer Encoding not supported");
    }

    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(rs);
    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

    return fromJson(object);
  }

  /**
   * @brief Process options from the prompt request
   *
   * This method processes the options from the prompt request and returns a JSON object
   * containing the options. It flattens the options into a JSON object, converting types
   * based on the type hint provided in the option.
   *
   * @param prompt The prompt request containing options to be processed
   */
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
  virtual Poco::JSON::Object toJson(const prompt::PromptRequest& prompt) = 0;

  /**
   * @brief Convert a JSON object to a prompt response
   *
   * This method converts a JSON object received from the prompt plugin into a prompt response.
   * It extracts the relevant fields from the JSON object and returns a PromptResponse object.
   *
   * @param object The JSON object to convert
   * @return A PromptResponse object containing the response data
   */
  virtual prompt::PromptResponse fromJson(const Poco::JSON::Object::Ptr object) = 0;

  std::string uri_;
  std::string method_;
  bool ssl_verify_;
  std::string auth_type_;
  std::string api_key_;
};

}  // namespace prompt
