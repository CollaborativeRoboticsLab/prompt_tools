#pragma once

#include <prompt_base/rest_base_class.hpp>
#include <prompt_base/utils/prompt_options.hpp>

namespace prompt
{
/**
 * @brief WhisperAPI
 *
 * This class implements a sentiment analysis prompt provider that uses Hugging Face's REST API.
 * It inherits from the BaseClass class and provides methods to initialize the provider,
 * convert prompt requests to JSON, and parse responses from JSON.
 *
 */
class WhisperAPI : public RestBaseClass
{
public:
  /**
   * @brief Constructor
   *
   * Initializes the WhisperAPI with default values.
   */
  WhisperAPI() : RestBaseClass()
  {
  }

  /**
   * @brief Initialize the sentiment analysis prompt provider
   *
   * This method initializes the provider with parameters from the ROS parameter server.
   *
   * @param node rclcpp::Node::SharedPtr ROS node
   */
  virtual void initialize(rclcpp::Node::SharedPtr node) override
  {
    // initialize base class
    initialize_rest_base(node, "WhisperAPI", "OPENAI_API_KEY");

    // declare parameters
    node_->declare_parameter("WhisperAPI.override_model_options", false);

    // get parameters from the parameter server
    override_ = node_->get_parameter("WhisperAPI.override_model_options").as_bool();

    if (override_)
    {
      RCLCPP_INFO(node_->get_logger(), "Overriding model options from parameters");
      prompt_options_ = prompt::load_from_paramters(node_, "WhisperAPI");
    }
    else
    {
      RCLCPP_INFO(node_->get_logger(), "Using model options from prompt request");
    }
  }

  /**
   * @brief Destructor
   *
   * Cleans up resources used by the WhisperAPI.
   */
  virtual ~WhisperAPI() = default;

  /**
   * @brief sendPrompt send a prompt to a prompt provider using REST
   *
   * Typical providers offer stream based responses which is also supported
   *
   * @param req The prompt request containing the prompt and options.
   * @return const PromptResponse
   */
  virtual prompt::PromptResponse sendPromptAudio(const prompt::PromptRequest& req) override
  {
    prompt::PromptResponse response;

    // create a new request
    prompt::PromptRequest request;
    request.prompt = req;

    // check if we should override model options if so do it
    if (override_)
      request.options = prompt_options_;

    // send the prompt request to the prompt_provider
    response = RestBaseClass::sendPromptAudio(request);

    return response;
  }

protected:
  /**
   * @brief convert a prompt request to a JSON object
   *
   * @param prompt prompt::PromptRequest the prompt request to convert
   * @return const Poco::JSON::Object
   */
  virtual Poco::JSON::Object toJson(const prompt::PromptRequest& prompt) override
  {
    throw prompt::PromptException("WhisperAPI does not support toJson conversion. check sendPromptAudio().");
  }

  /**
   * @brief convert a JSON object to a prompt response
   *
   * @param object Poco::JSON::Object::Ptr the JSON object to convert
   * @return const prompt::PromptResponse
   */
  virtual prompt::PromptResponse fromJson(const Poco::JSON::Object::Ptr object) override
  {
    prompt::PromptResponse res;

    // Extract "text" from the root object
    if (object->has("text"))
    {
      res.response = object->getValue<std::string>("text");
    }

    return res;
  }

  /**
   * @brief Override model options
   *
   * This flag indicates whether to override the model options from the parameters or not.
   */
  bool override_;

  /**
   * @brief Model options
   *
   * This is the model options used by the prompt provider.
   */
  std::vector<prompt::PromptOption> prompt_options_;
};

}  // namespace prompt
