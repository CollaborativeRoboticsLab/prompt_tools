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
#include <prompt_base/utils/prompt_options.hpp>
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
  /**
   * @brief Constructor
   *
   * Initializes the RestBaseClass with default values.
   */
  RestBaseClass() : BaseClass()
  {
  }

  /**
   * @brief Destructor
   *
   * Cleans up resources used by the RestBaseClass.
   */
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

    // declare parameters only if not already declared (idempotent init)
    if (!node_->has_parameter(plugin_name_ + ".rest.method"))
    {
      node_->declare_parameter(plugin_name_ + ".rest.method", "POST");
    }
    if (!node_->has_parameter(plugin_name_ + ".rest.ssl_verify"))
    {
      node_->declare_parameter(plugin_name_ + ".rest.ssl_verify", true);
    }
    if (!node_->has_parameter(plugin_name_ + ".rest.auth_type"))
    {
      node_->declare_parameter(plugin_name_ + ".rest.auth_type", "Bearer");
    }

    // get parameters from the parameter server
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
    else
    {
      RCLCPP_INFO(node_->get_logger(), "An API key is not used for this plugin, using empty string");
      api_key_ = "";
    }

    // log the parameters
    RCLCPP_INFO(node_->get_logger(), "%s Method: %s", plugin_name_.c_str(), method_.c_str());
    RCLCPP_INFO(node_->get_logger(), "%s SSL Verify: %s", plugin_name_.c_str(), ssl_verify_ ? "true" : "false");
    RCLCPP_INFO(node_->get_logger(), "%s Auth Type: %s", plugin_name_.c_str(), auth_type_.c_str());
    RCLCPP_INFO(node_->get_logger(), "%s Plugin initialized", plugin_name_.c_str());

    RCLCPP_INFO(node_->get_logger(), "Loading default model options from parameters.");
    required_options_ = prompt::load_from_parameters(node_, plugin_name_);
  }

protected:
  /**
   * @brief Process the HTTP request and return the response as a JSON object
   *
   * This method sends the HTTP request to the specified URI and returns the response
   * as a Poco::JSON::Object::Ptr.
   *
   * @param body_json The JSON object containing the request body.
   * @param uri The URI to send the request to.
   * @return Poco::JSON::Object::Ptr The JSON object containing the response.
   * @throws prompt::PromptException if there is an error during the request.
   */
  Poco::JSON::Object::Ptr process(Poco::JSON::Object& body_json, std::string& uri)
  {
    // convert the uri
    Poco::URI uri_obj(uri);

    // calculate body length
    std::ostringstream body_stream;
    body_json.stringify(body_stream);

    // create request object
    Poco::Net::HTTPRequest request(method_, uri_obj.getPath());
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
    if (uri_obj.getScheme() == "https")
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
      session_ptr = std::make_unique<Poco::Net::HTTPSClientSession>(uri_obj.getHost(), uri_obj.getPort(), context);
      RCLCPP_DEBUG(node_->get_logger(), "secure session created");
    }
    else
    {
      // create insecure session
      session_ptr = std::make_unique<Poco::Net::HTTPClientSession>(uri_obj.getHost(), uri_obj.getPort());

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
    return result.extract<Poco::JSON::Object::Ptr>();
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
  virtual const Poco::JSON::Object handle_options(std::vector<prompt::PromptOption>& options)
  {
    // flatten options into object
    Poco::JSON::Object result;

    for (const prompt::PromptOption& option : options)
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

  std::string method_;
  bool ssl_verify_;
  std::string auth_type_;

  /**
   * @brief API key
   */
  std::string api_key_;

  /**
   * @brief Model options
   *
   * This is the model options used by the prompt provider by default in case model options are not
   * provided in the request
   */
  std::vector<prompt::PromptOption> required_options_;
};

}  // namespace prompt
