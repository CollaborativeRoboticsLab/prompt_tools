#pragma once

#include <prompt_base/base_class.hpp>
#include <prompt_base/utils/prompt_options.hpp>
#include <string>

namespace prompt
{
/**
 * @brief TokenizeBaseClass
 *
 * This class implements a base class for RESTful prompt plugins.
 * It provides a common interface for sending prompts and receiving responses
 */
class TokenizeBaseClass : public BaseClass
{
public:
  /**
   * @brief Constructor
   *
   * Initializes the TokenizeBaseClass with default values.
   */
  TokenizeBaseClass() : BaseClass()
  {
  }

  /**
   * @brief Destructor
   *
   * Cleans up resources used by the TokenizeBaseClass.
   */
  virtual ~TokenizeBaseClass() = default;

  /**
   * @brief Initialize the REST base class
   *
   * This method initializes the REST base class with parameters from the ROS parameter server.
   *
   * @param node rclcpp::Node::SharedPtr ROS node
   * @param plugin_name std::string Name of the plugin, used for parameter names
   *
   * @throws prompt::EmbedException if there is an error during initialization
   */
  virtual void initialize_tokenize_base(rclcpp::Node::SharedPtr node, std::string plugin_name = "TokenizeBaseClass")
  {
    // initialize base class
    initialize_base(node, plugin_name);

    required_options_ = prompt::load_from_parameters(node_, plugin_name_);

    RCLCPP_INFO(node_->get_logger(), "%s Plugin initialized", plugin_name_.c_str());
  }

  /**
   * @brief get_tokens sends a text to a token provider
   *
   * @param req The token request containing the text and options.
   * @return const TokenResponse
   */
  virtual prompt::TokenResponse get_tokens(prompt::TokenRequest& req) override
  {
    // verify and add required model options
    prompt::ensure_model_options(node_, req.options, required_options_);

    // create prompt provider response container
    prompt::TokenResponse res = process(req);

    return res;
  }

protected:
  /**
   * @brief Process a tokenize request
   *
   * This method processes the tokenize request and returns a TokenResponse. Should be
   * implemented by derived classes.
   *
   * @param input The tokenize request to convert
   * @return A TokenResponse containing the tokens and options
   */
  virtual prompt::TokenResponse process(prompt::TokenRequest& input) = 0;

  /**
   * @brief Model options
   *
   * This is the model options used by the prompt provider by default in case model options are not
   * provided in the request
   */
  std::vector<prompt::PromptOption> required_options_;
};

}  // namespace prompt
