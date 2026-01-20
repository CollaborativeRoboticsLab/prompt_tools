#pragma once

#include <prompt_base/tokenize_base_class.hpp>
#include "encoding.h"

namespace prompt
{

/**
 * @brief OpenAITokenize
 *
 *
 */
class OpenAITokenize : public TokenizeBaseClass
{
public:
  /**
   * @brief Construct a new OpenAI Tokenize Provider object
   */
  OpenAITokenize() : TokenizeBaseClass()
  {
  }

  /**
   * @brief Initialize the OpenAITokenize
   *
   * This method initializes the OpenAITokenize with parameters from the ROS parameter server.
   *
   * @param node rclcpp::Node::SharedPtr ROS node
   */
  virtual void initialize(rclcpp::Node::SharedPtr node) override
  {
    // initialize base class
    initialize_tokenize_base(node, "OpenAITokenize");
  }

protected:
  /**
   * @brief Convert a tokenize request to a JSON object
   *
   * This method converts the tokenize request to a JSON object that can be sent to the tokenize plugin.
   * It includes the tokenize text and options in the JSON object.
   *
   * @param input The tokenize request to convert
   * @return A JSON object representing the tokenize request
   */
  virtual prompt::TokenResponse process(prompt::TokenRequest& input) override
  {
    // Initialize the reponse
    prompt::TokenResponse res;

    //find the 'model' option from the modeloptions
    std::string model_name;

    for (const auto& option : input.options)
    {
      if (option.key == "model")
      {
        model_name = option.value;
        break;
      }
    }

    if (model_name=="O200K_BASE")
    {
      encoder_ = GptEncoding::get_encoding(LanguageModel::O200K_BASE);
    }
    else if ((model_name=="CL100K_BASE"))
    {
      encoder_ = GptEncoding::get_encoding(LanguageModel::CL100K_BASE);
    }
    else if (model_name=="R50K_BASE")
    {
      encoder_ = GptEncoding::get_encoding(LanguageModel::R50K_BASE);
    }
    else if (model_name=="P50K_BASE")
    {
      encoder_ = GptEncoding::get_encoding(LanguageModel::P50K_BASE);
    }
    else if (model_name=="P50K_EDIT")
    {
      encoder_ = GptEncoding::get_encoding(LanguageModel::P50K_EDIT);
    }
    else
    {
      // Add as error log
      res.success = false;
      res.error = "OpenAITokenize: Unsupported model for tokenization: " + model_name;

      RCLCPP_ERROR(node_->get_logger(), res.error.c_str());
      throw std::runtime_error(res.error);
    }

    if (input.encode)
    {
      // Tokenize the input text
      res.tokens = encoder_->encode(input.text);
      res.success = true;
    }
    else
    {
      // Decode the input tokens
      res.text = encoder_->decode(input.tokens);
      res.success = true;
    }

    return res;
  }

  std::shared_ptr<GptEncoding> encoder_;
};

}  // namespace prompt
