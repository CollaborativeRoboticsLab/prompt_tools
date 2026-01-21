#pragma once

#include <prompt_base/embed_base_class.hpp>

namespace prompt
{

/**
 * @brief OpenAIEmbedding
 * 
 * This class implements an OpenAI embedding provider plugin. 
 * It provides methods to initialize the plugin, convert embed requests to JSON,
 * and parse JSON responses into embed responses.
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
    try
    {
      // Embeddings API returns 'data' array with one or more items containing 'embedding' and 'index'
      if (object->has("data"))
      {
        Poco::JSON::Array::Ptr data_array = object->getArray("data");
        for (size_t di = 0; di < data_array->size(); ++di)
        {
          Poco::JSON::Object::Ptr embed_obj = data_array->getObject(di);
          prompt::Embedding emb;

          if (embed_obj->has("embedding"))
          {
            if (embed_obj->isArray("embedding"))
            {
              Poco::JSON::Array::Ptr arr = embed_obj->getArray("embedding");
              emb.float_embedding.reserve(arr->size());
              for (size_t i = 0; i < arr->size(); ++i)
              {
                emb.float_embedding.push_back(static_cast<float>(arr->get(i).convert<double>()));
              }
              emb.is_float = true;
            }
            else
            {
              // Treat as base64 string
              emb.base64_embedding = embed_obj->getValue<std::string>("embedding");
              emb.is_float = false;
            }
          }

          if (embed_obj->has("index"))
          {
            emb.index = embed_obj->getValue<int>("index");
          }

          res.embeddings.push_back(emb);
        }
      }

      // Top-level metadata
      if (object->has("model"))
      {
        res.model = object->getValue<std::string>("model");
      }
      if (object->has("usage"))
      {
        try
        {
          Poco::JSON::Object::Ptr usage = object->getObject("usage");
          if (usage->has("prompt_tokens"))
          {
            res.prompt_tokens = usage->getValue<int>("prompt_tokens");
          }
          if (usage->has("total_tokens"))
          {
            res.total_tokens = usage->getValue<int>("total_tokens");
          }
        }
        catch (const Poco::Exception& ex)
        {
          RCLCPP_WARN(node_->get_logger(), "Failed to parse usage: %s", ex.what());
        }
      }

      res.success = !res.embeddings.empty();
    }
    catch (const Poco::Exception& ex)
    {
      RCLCPP_WARN(node_->get_logger(), "Failed to parse OpenAI embeddings response: %s", ex.what());
      res.error = ex.what();
      res.success = false;
    }
    return res;
  }
};

}  // namespace prompt
