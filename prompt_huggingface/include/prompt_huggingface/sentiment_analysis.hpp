#pragma once

// include poco json and net/netssl
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/NetException.h>
#include <Poco/URI.h>

#include <rclcpp/rclcpp.hpp>
#include <prompt_msgs/msg/prompt.hpp>
#include <prompt_base/utils/exceptions.hpp>
#include <prompt_base/utils/structs.hpp>
#include <prompt_base/rest_base_class.hpp>

namespace prompt
{
/**
 * @brief SentimentAnalysis
 *
 * This class implements a sentiment analysis prompt provider that uses Hugging Face's REST API.
 * It inherits from the BaseClass class and provides methods to initialize the provider,
 * convert prompt requests to JSON, and parse responses from JSON.
 *
 */
class SentimentAnalysis : public RestBaseClass
{
public:
  /**
   * @brief Constructor
   *
   * Initializes the SentimentAnalysis with default values.
   */
  SentimentAnalysis() : RestBaseClass()
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
    initialize_rest_base(node, "SentimentAnalysis", "HUGGINGFACE_API_KEY");
  }

  /**
   * @brief Destructor
   *
   * Cleans up resources used by the SentimentAnalysis.
   */
  virtual ~SentimentAnalysis() = default;

protected:
  /**
   * @brief convert a prompt request to a JSON object
   *
   * @param prompt prompt::PromptRequest the prompt request to convert
   * @return const Poco::JSON::Object
   */
  virtual Poco::JSON::Object toJson(const prompt::PromptRequest& prompt) override
  {
    // add options
    Poco::JSON::Object result = handle_options(prompt);

    // add prompt
    result.set("inputs", prompt.prompt);

    return result;
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

    // Check if the root contains a "data" array-style structure (wrapped JSON result)
    if (object->has("data"))
    {
      Poco::Dynamic::Var dataVar = object->get("data");

      if (dataVar.type() == typeid(Poco::JSON::Array::Ptr))
      {
        auto outerArray = dataVar.extract<Poco::JSON::Array::Ptr>();
        if (!outerArray->empty())
        {
          auto inner = outerArray->getArray(0);
          for (size_t i = 0; i < inner->size(); ++i)
          {
            auto item = inner->getObject(i);
            std::string label = item->getValue<std::string>("label");
            double score = item->getValue<double>("score");

            // You can use the first label as the main response
            if (i == 0)
              res.response = label;
              res.confidence = score;
          }
        }
      }
    }

    return res;
  }
};

}  // namespace prompt
