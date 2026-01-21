#pragma once

#include <prompt_base/rest_base_class.hpp>
#include <prompt_base/utils/prompt_options.hpp>
#include <string>

namespace prompt
{

/**
 * @brief EmbedBaseClass
 *
 * This class implements a base class for RESTful embedding plugins.
 * It provides a common interface for sending embedding requests and receiving responses
 */
class EmbedBaseClass : public RestBaseClass
{
public:
  /**
   * @brief Constructor
   *
   * Initializes the EmbedBaseClass with default values.
   */
  EmbedBaseClass() : RestBaseClass(), embed_uri_("")
  {
  }

  /**
   * @brief Destructor
   *
   * Cleans up resources used by the EmbedBaseClass.
   */
  virtual ~EmbedBaseClass() = default;

  /**
   * @brief Initialize the REST base class
   *
   * This method initializes the REST base class with parameters from the ROS parameter server.
   *
   * @param node rclcpp::Node::SharedPtr ROS node
   * @param plugin_name std::string Name of the plugin, used for parameter names
   * @param api_key_name std::string Name of the API key parameter, if different from the default
   * @throws prompt::EmbedException if there is an error during initialization
   */
  virtual void initialize_embed_base(rclcpp::Node::SharedPtr node, std::string plugin_name = "EmbedBaseClass",
                                     std::string api_key_name = "")
  {
    // initialize base class
    initialize_rest_base(node, plugin_name, api_key_name);

    // declare parameters only if not already declared (idempotent init)
    if (!node_->has_parameter(plugin_name_ + ".rest.embedding_uri"))
    {
      node_->declare_parameter(plugin_name_ + ".rest.embedding_uri", "");
    }

    // get parameters from the parameter server
    embed_uri_ = node_->get_parameter(plugin_name_ + ".rest.embedding_uri").as_string();

    // log the parameters
    RCLCPP_INFO(node_->get_logger(), "%s Embedding URI: %s", plugin_name_.c_str(), embed_uri_.c_str());
  }

  /**
   * @brief get_embeddings sends a text to an embedding provider using REST
   *
   * @param req The embed request containing the text and options.
   * @return const EmbedResponse
   */
  virtual prompt::EmbedResponse get_embeddings(prompt::EmbedRequest& req) override
  {
    // verify and add required model options
    prompt::ensure_model_options(node_, req.options, required_options_);

    // prepare request body
    Poco::JSON::Object body_json = toJson(req);

    // process the prompt
    Poco::JSON::Object::Ptr object = process(body_json, embed_uri_);

    // create prompt provider response container
    prompt::EmbedResponse res = fromJson(object);

    return res;
  }

protected:
  /**
   * @brief Convert a embed request to a JSON object
   *
   * This method converts the embed request to a JSON object that can be sent to the embed plugin.
   * It includes the embed text and options in the JSON object.
   *
   * @param input The embed request to convert
   * @return A JSON object representing the embed request
   */
  virtual Poco::JSON::Object toJson(prompt::EmbedRequest& input) = 0;

  /**
   * @brief Convert a JSON object to a embed response
   *
   * This method converts a JSON object received from the embed plugin into a embed response.
   * It extracts the relevant fields from the JSON object and returns a EmbedResponse object.
   *
   * @param object The JSON object to convert
   * @return A EmbedResponse object containing the response data
   */
  virtual prompt::EmbedResponse fromJson(const Poco::JSON::Object::Ptr object) = 0;

  /**
   * @brief URI for embedding requests
   */
  std::string embed_uri_;
};

}  // namespace prompt
