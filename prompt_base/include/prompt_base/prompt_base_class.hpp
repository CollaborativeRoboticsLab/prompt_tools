#pragma once

#include <prompt_base/rest_base_class.hpp>
#include <prompt_base/utils/prompt_options.hpp>
#include <string>

namespace prompt
{

/**
 * @brief PromptBaseClass
 *
 * This class implements a base class for RESTful prompt plugins.
 * It provides a common interface for sending prompts and receiving responses
 */
class PromptBaseClass : public RestBaseClass
{
public:
  /**
   * @brief Constructor
   *
   * Initializes the RestBaseClass with default values.
   */
  PromptBaseClass() : RestBaseClass(), uri_(""), chat_uri_("")
  {
  }

  /**
   * @brief Destructor
   *
   * Cleans up resources used by the RestBaseClass.
   */
  virtual ~PromptBaseClass() = default;

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
  virtual void initialize_prompt_base(rclcpp::Node::SharedPtr node, std::string plugin_name = "PromptBaseClass",
                                      std::string api_key_name = "")
  {
    // initialize base class
    initialize_rest_base(node, plugin_name, api_key_name);

    // declare parameters only if not already declared (idempotent init)
    if (!node_->has_parameter(plugin_name_ + ".rest.uri"))
    {
      node_->declare_parameter(plugin_name_ + ".rest.uri", "");
    }
    if (!node_->has_parameter(plugin_name_ + ".rest.chat_uri"))
    {
      node_->declare_parameter(plugin_name_ + ".rest.chat_uri", "");
    }

    // get parameters from the parameter server
    uri_ = node_->get_parameter(plugin_name_ + ".rest.uri").as_string();
    chat_uri_ = node_->get_parameter(plugin_name_ + ".rest.chat_uri").as_string();

    // log the parameters
    RCLCPP_INFO(node_->get_logger(), "%s Single URI: %s", plugin_name_.c_str(), uri_.c_str());
    RCLCPP_INFO(node_->get_logger(), "%s Chat URI: %s", plugin_name_.c_str(), chat_uri_.c_str());
  }

  /**
   * @brief sendPrompt send a prompt to a prompt provider using REST
   *
   * Typical providers offer stream based responses which is also supported
   *
   * @param req The prompt request containing the prompt and options.
   * @return const PromptResponse
   */
  virtual prompt::PromptResponse sendPrompt(prompt::PromptRequest& req)
  {
    // verify and add required model options
    prompt::ensure_model_options(node_, req.options, required_options_);

    // prepare request body
    Poco::JSON::Object body_json = toJson(req);

    // process the prompt
    Poco::JSON::Object::Ptr object = process(body_json, uri_);

    // create prompt provider response container
    prompt::PromptResponse res = fromJson(object);

    return res;
  }

  /**
   * @brief sendConversation send a prompt with conversation history to a prompt provider using REST
   *
   * Typical providers offer stream based responses which is also supported
   *
   * @param latest_request The prompt request containing the prompt and options.
   * @param conversation The conversation history for context.
   * @return const PromptResponse
   */
  virtual prompt::PromptResponse sendConversation(prompt::PromptRequest& latest_request,
                                                  std::vector<PromptDialogue>& conversation)
  {
    // verify and add required model options
    prompt::ensure_model_options(node_, latest_request.options, required_options_);

    // prepare request body
    Poco::JSON::Object body_json = toJsonConversation(latest_request, conversation);

    // process the prompt
    Poco::JSON::Object::Ptr object = process(body_json, chat_uri_);

    // create prompt provider response container
    prompt::PromptResponse res = fromJsonConversation(object);

    return res;
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
  virtual Poco::JSON::Object toJson(prompt::PromptRequest& prompt) = 0;

  /**
   * @brief Convert a prompt request with conversation history to a JSON object
   *
   * This method converts the prompt request and conversation history to a JSON object that can be sent to the prompt
   * plugin. It includes the prompt text, options, and conversation history in the JSON object.
   *
   * @param latest_request The latest prompt request to convert
   * @param conversation The conversation history to include
   * @return A JSON object representing the prompt request with conversation history
   */
  virtual Poco::JSON::Object toJsonConversation(prompt::PromptRequest& latest_request,
                                                std::vector<PromptDialogue>& conversation) = 0;

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
  virtual prompt::PromptResponse fromJsonConversation(const Poco::JSON::Object::Ptr object) = 0;

  /**
   * @brief URI for single prompt requests
   */
  std::string uri_;

  /**
   * @brief URI for chat/conversation requests
   */
  std::string chat_uri_;
};

}  // namespace prompt
