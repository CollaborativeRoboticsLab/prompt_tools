#pragma once

#include <prompt_base/rest_base_class.hpp>

namespace prompt
{

/**
 * @brief OpenAIProvider
 *
 * This is a prompt provider that uses a REST API to send and receive prompts
 * the typical rest api uses application/json content type so that is what is
 * supported
 *
 */
class OpenAIEmbedding : public EmbedBaseClass
{
public:
  /**
   * @brief Construct a new OpenAI Embedding Provider object
   */
  OpenAIEmbedding() : EmbedBaseClass()
  {
  }

  /**
   * @brief Initialize the OpenAIEmbedding
   *
   * This method initializes the OpenAIEmbedding with parameters from the ROS parameter server.
   *
   * @param node rclcpp::Node::SharedPtr ROS node
   */
  virtual void initialize(rclcpp::Node::SharedPtr node) override
  {
    // initialize base class
    initialize_embed_base(node, "OpenAIEmbedding", "OPENAI_API_KEY");
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
  virtual Poco::JSON::Object toJson(prompt::EmbedRequest& input) override
  {
    // add options (model, temperature, etc.)
    Poco::JSON::Object result = handle_options(input.options);

    // /v1/responses uses `input` to send the text directly as a string.
    result.set("input", input.text );
    return result;
  }

  /**
   * @brief Convert a JSON object to a prompt response for OpenAI Embeddings
   *
   * This method converts a JSON object received from the embeddings endpoint into a PromptResponse.
   * It extracts the embedding vector and other metadata.
   *
   * @param object The JSON object to convert
   * @return A PromptResponse containing the embedding and options
   */
  virtual prompt::EmbedResponse fromJson(const Poco::JSON::Object::Ptr object) override
  {
    prompt::EmbedResponse res;
    // Embeddings API returns 'data' array with 'embedding' field
    if (object->has("data"))
    {
      try
      {
        Poco::JSON::Array::Ptr data_array = object->getArray("data");
        if (!data_array->empty())
        {
          Poco::JSON::Object::Ptr embed_obj = data_array->getObject(0);
          if (embed_obj->has("embedding"))
          {
            // Try to detect type: float array or base64 string
            if (embed_obj->isArray("embedding"))
            {
              Poco::JSON::Array::Ptr embedding = embed_obj->getArray("embedding");
              res.float_vector.clear();
              for (size_t i = 0; i < embedding->size(); ++i)
              {
                res.float_embedding.push_back(static_cast<float>(embedding->get(i).convert<double>()));
              }
              res.embed_type = prompt::EmbedType::Float;
              res.success = !res.float_embedding.empty();
            }
            else if (embed_obj->isString("embedding"))
            {
              res.base64_embedding = embed_obj->getValue<std::string>("embedding");
              res.embed_type = prompt::EmbedType::Base64;
              res.success = !res.base64_embedding.empty();
            }
          }
        }
      }
      catch (const Poco::Exception& ex)
      {
        RCLCPP_WARN(node_->get_logger(), "Failed to parse OpenAI embeddings response: %s", ex.what());
        res.error = ex.what();
      }
    }
    // Attach remaining top-level fields as options (excluding large nested structures)
    for (Poco::JSON::Object::ConstIterator it = object->begin(); it != object->end(); ++it)
    {
      if ((it->first == "data") || (it->first == "usage"))
        continue;
      try
      {
        if (!it->second.isEmpty() && it->second.isString())
        {
          res.options.push_back(prompt::PromptOption{ it->first, it->second.convert<std::string>(), "" });
        }
        else if (!it->second.isEmpty())
        {
          res.options.push_back(prompt::PromptOption{ it->first, it->second.toString(), "" });
        }
        else
        {
          res.options.push_back(prompt::PromptOption{ it->first, "[null]", "" });
        }
      }
      catch (const Poco::Exception& ex)
      {
        RCLCPP_WARN(node_->get_logger(), "Failed to convert JSON key '%s': %s", it->first.c_str(), ex.what());
      }
    }
    return res;
  }
};

}  // namespace prompt
