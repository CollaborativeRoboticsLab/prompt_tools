#pragma once

#include <uuid/uuid.h>

#include <functional>
#include <memory>
#include <pluginlib/class_loader.hpp>
#include <prompt_base/base_class.hpp>
#include <prompt_base/utils/conversions.hpp>
#include <prompt_base/utils/exceptions.hpp>
#include <prompt_msgs/msg/prompt_history.hpp>
#include <prompt_msgs/msg/prompt_transaction.hpp>
#include <prompt_msgs/srv/prompt.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <thread>

namespace prompt
{
/**
 * @brief PromptBridge
 *
 * This class provides a bridge between ROS resources and the prompt provider and scheme
 * It also provides a ROS action and service servers that can be used to send and receive prompts
 *
 */
class PromptBridge : public rclcpp::Node
{
  using PromptSrv = prompt_msgs::srv::Prompt;

public:
  /**
   * @brief Construct a new Prompt Bridge object
   *
   * @param options Node options for the PromptBridge node
   */
  PromptBridge(const rclcpp::NodeOptions& options = rclcpp::NodeOptions())
    : Node("prompt_bridge", options), prompt_provider_loader_("prompt_bridge", "prompt::BaseClass")
  {
    try
    {
      if (shared_from_this())
      {
        initialize();
      }
    }
    catch (const std::bad_weak_ptr&)
    {
      // Not yet safe — probably standalone without make_shared
    }
  }

  /**
   * @brief Initialize the PromptBridge node
   *
   * This method initializes the PromptBridge node by declaring parameters and loading plugins.
   */
  void initialize()
  {
    /*************************************************************************
     * Declare parameters
     ************************************************************************/

    this->declare_parameter("frame_id", "agent");
    this->declare_parameter("cached_transactions", 10);

    frame_id_ = this->get_parameter("frame_id").as_string();
    transaction_limit_ = this->get_parameter("cached_transactions").as_int();

    /*************************************************************************
     * load model family plugins
     ************************************************************************/

    this->declare_parameter("model_family_names", rclcpp::ParameterValue(std::vector<std::string>{}));
    model_families_keys_ = this->get_parameter("model_family_names").as_string_array();

    for (const auto& family_key : model_families_keys_)
    {
      this->declare_parameter("model_family_plugins." + family_key, rclcpp::ParameterValue(""));
      std::string plugin = this->get_parameter("model_family_plugins." + family_key).as_string();
      model_families_names_[family_key] = plugin;
    }

    /*************************************************************************
     * prompt history ros interfaces
     ************************************************************************/

    // history publisher
    prompt_history_pub_ = this->create_publisher<prompt_msgs::msg::PromptHistory>("prompt/history", 1);

    // history publisher timer
    history_pub_timer_ =
        this->create_wall_timer(std::chrono::duration<double>(1.0), std::bind(&PromptBridge::history_timer, this));

    /*************************************************************************
     * prompt service ros interfaces
     ************************************************************************/

    prompt_service_ =
        this->create_service<PromptSrv>("prompt/prompt", std::bind(&PromptBridge::prompt_service_cb, this,
                                                                   std::placeholders::_1, std::placeholders::_2));

    RCLCPP_INFO(this->get_logger(), "Prompt service created at 'prompt/prompt'");
    RCLCPP_INFO(this->get_logger(), "PromptBridge initialized");
  }

  ~PromptBridge() = default;

  /**
   * @brief Generate a UUID string for prompt tracking
   */
  static const std::string generate_uuid()
  {
    uuid_t uuid;
    uuid_generate_random(uuid);
    char uuid_str[40];
    uuid_unparse(uuid, uuid_str);
    return std::string(uuid_str);
  }

  std::shared_ptr<prompt::BaseClass> load_model(std::string model_family)
  {
    std::shared_ptr<prompt::BaseClass> prompt_provider_instance_;

    // check if the model family exists
    if (model_families_names_.find(model_family) != model_families_names_.end())
    {
      prompt_provider_instance_ = prompt_provider_loader_.createSharedInstance(model_families_names_[model_family]);
      prompt_provider_instance_->initialize(shared_from_this());

      return prompt_provider_instance_;
    }
    else
    {
      RCLCPP_ERROR(this->get_logger(), "Model family not found");
      throw prompt::PromptException("Model family not found");
    }
  }

  /**
   * @brief prompt service callback for instance prompts
   *
   * This function is called when a prompt service request is received. It processes the prompt request with a new
   * instance of the prompt provider and allows for concurrent prompt handling. It processes the prompt request
   * using the prompt provider, and sends the response back to the client.
   *
   * @param req the prompt service request. contains a prompt message, which is processed by the prompt provider
   * @param res the prompt service response. contains the processed prompt response.
   *
   * @throws prompt::PromptException if the prompt provider fails to process the prompt
   */
  void prompt_service_cb(const std::shared_ptr<PromptSrv::Request> req, std::shared_ptr<PromptSrv::Response> res)
  {
    // prompt provider
    std::shared_ptr<prompt::BaseClass> prompt_provider_;
    try
    {
      prompt_provider_ = load_model(req->prompt.model_family);
    }
    catch (const prompt::PromptException& e)
    {
      RCLCPP_ERROR(this->get_logger(), "Failed to load model: %s", e.what());
      return;
    }
    catch (const std::exception& e)
    {
      RCLCPP_ERROR(this->get_logger(), "Unexpected error while loading model: %s", e.what());
      return;
    }

    std::string uuid;

    prompt::PromptRequest input = prompt::fromMsg(req->prompt);
    prompt::PromptResponse result;

    // pre send time
    auto pre_send_time = this->now();

    // check if chat mode is enabled
    if (req->prompt.use_chat_mode)
    {
      // chat mode is enabled. requests and response are part of a conversation and they are stored accordingly.
      // check if this is a new prompt or a continuation of a previous prompt
      if (req->uuid == "")
      {
        // since uuid is empty, This is a new prompt request. Check if caching is requested.
        if (req->prompt.use_cache)
        {
          // new prompt with caching requested. check if flushing cache is requested
          if (req->prompt.flush_cache)
          {
            // flushing cache on new prompt doesn't make sense, log a warning and process the prompt directly
            RCLCPP_WARN(this->get_logger(), "Flushing cache requested on new prompt with no uuid. Prompt will be "
                                            "processed without caching.");

            // continue caching, generate a new uuid for tracking
            result = prompt_provider_->sendPrompt(input);
            result.buffered = false;

            // generate a new uuid for tracking since chat mode is enabled
            uuid = generate_uuid();

            // store the user prompt in the conversation history
            PromptDialogue dialogue;
            dialogue.role = "user";
            dialogue.content = req->prompt.prompt;
            prompt_conversations_[uuid].push_back(dialogue);

            // store the assistant response in the conversation history
            PromptDialogue dialogue2;
            dialogue2.role = "assistant";
            dialogue2.content = result.response;
            prompt_conversations_[uuid].push_back(dialogue2);

            // convert the result to message and return the uuid to the client for future reference
            res->response = prompt::toMsg(result);
            res->uuid = uuid;
            RCLCPP_INFO(this->get_logger(), "New chat prompt processed. Generated UUID: %s", uuid.c_str());
          }
          else
          {
            // continue caching, generate a new uuid for tracking
            uuid = generate_uuid();

            PromptDialogue dialogue;
            dialogue.role = "user";
            dialogue.content = req->prompt.prompt;
            prompt_conversations_[uuid].push_back(dialogue);

            result.buffered = true;

            // convert the result to message and return the uuid to the client for future reference
            res->response = prompt::toMsg(result);
            res->uuid = uuid;

            RCLCPP_INFO(this->get_logger(), "New chat prompt cached. Generated UUID: %s", uuid.c_str());
          }
        }
        else
        {
          // no caching, process the prompt directly
          // generate a new uuid for tracking since chat mode is enabled
          uuid = generate_uuid();

          // no caching required, process the prompt directly
          result = prompt_provider_->sendPrompt(input);
          result.buffered = false;

          // store the user prompt in the conversation history
          PromptDialogue dialogue;
          dialogue.role = "user";
          dialogue.content = req->prompt.prompt;
          prompt_conversations_[uuid].push_back(dialogue);

          // store the assistant response in the conversation history
          PromptDialogue dialogue2;
          dialogue2.role = "assistant";
          dialogue2.content = result.response;
          prompt_conversations_[uuid].push_back(dialogue2);

          // convert the result to message and return the uuid to the client for future reference
          res->response = prompt::toMsg(result);
          res->uuid = uuid;

          RCLCPP_INFO(this->get_logger(), "New chat prompt processed. Generated UUID: %s", uuid.c_str());
        }
      }
      else
      {
        // Since uuid is provided, this is a continuation of a previous prompt request
        if (req->prompt.use_cache)
        {
          if (req->prompt.flush_cache)
          {
            // flushing the cache, retrieve the conversation history using the provided uuid
            uuid = req->uuid;

            // prompt the provider with the conversation history
            result = prompt_provider_->sendConversation(input, prompt_conversations_[uuid]);
            result.buffered = false;

            // store the user prompt in the conversation history
            PromptDialogue dialogue;
            dialogue.role = "user";
            dialogue.content = req->prompt.prompt;
            prompt_conversations_[uuid].push_back(dialogue);

            // store the assistant response in the conversation history
            PromptDialogue dialogue2;
            dialogue2.role = "assistant";
            dialogue2.content = result.response;
            prompt_conversations_[uuid].push_back(dialogue2);

            // convert the result to message and return the uuid to the client for future reference
            res->response = prompt::toMsg(result);
            res->uuid = uuid;
            RCLCPP_INFO(this->get_logger(), "Chat prompt processed with flushed cache. UUID: %s", uuid.c_str());
          }
          else
          {
            // continue caching, retrieve the conversation history using the provided uuid
            uuid = req->uuid;

            // find the last dialogue related to the uuid from conversation and update the prompt in order to cache
            prompt_conversations_[uuid].back().content += " " + req->prompt.prompt;
            result.buffered = true;

            // convert the result to message and return the uuid to the client for future reference
            res->response = prompt::toMsg(result);
            res->uuid = uuid;

            RCLCPP_INFO(this->get_logger(), "Chat prompt cached. UUID: %s", uuid.c_str());
          }
        }
        else
        {
          // no caching, process the prompt directly.
          // retrieve the conversation history using the provided uuid
          uuid = req->uuid;

          // prompt the provider with the conversation history
          result = prompt_provider_->sendConversation(input, prompt_conversations_[uuid]);
          result.buffered = false;

          // store the user prompt in the conversation history
          PromptDialogue dialogue;
          dialogue.role = "user";
          dialogue.content = req->prompt.prompt;
          prompt_conversations_[uuid].push_back(dialogue);

          // store the assistant response in the conversation history
          PromptDialogue dialogue2;
          dialogue2.role = "assistant";
          dialogue2.content = result.response;
          prompt_conversations_[uuid].push_back(dialogue2);

          // convert the result to message and return the uuid to the client for future reference
          res->response = prompt::toMsg(result);
          res->uuid = uuid;

          RCLCPP_INFO(this->get_logger(), "Chat prompt processed. UUID: %s", uuid.c_str());
        }
      }
    }
    else
    {
      // chat mode is not enabled, process the prompt individually. check if this is a new prompt or
      // a continuation of caching of a previous prompt. Since chat mode is not enabled, caching is only
      // used to collect inputs from multiple sources and only containes the role of "user".
      if (req->uuid == "")
      {
        // since uuid is empty, This is a new prompt request. Check if caching is requested.
        if (req->prompt.use_cache)
        {
          if (req->prompt.flush_cache)
          {
            // flushing cache on new prompt doesn't make sense, log a warning and process prompt imidiately
            RCLCPP_WARN(this->get_logger(), "Flushing cache requested on new prompt with no uuid. Prompt will be "
                                            "processed without caching.");

            // continue caching, generate a new uuid for tracking
            result = prompt_provider_->sendPrompt(input);
            result.buffered = false;

            res->response = prompt::toMsg(result);
            res->uuid = "";
            RCLCPP_INFO(this->get_logger(), "New chat prompt processed. No UUID generated due to flush request and "
                                            "disabled chat mode");
          }
          else
          {
            // no flush request thus continue caching, since uuid is not provided, generate a new uuid for tracking
            uuid = generate_uuid();

            PromptDialogue dialogue;
            dialogue.role = "user";
            dialogue.content = req->prompt.prompt;

            prompt_conversations_[uuid].push_back(dialogue);

            result.buffered = true;

            // convert the result to message and return the uuid to the client for future reference
            res->response = prompt::toMsg(result);
            res->uuid = uuid;

            RCLCPP_INFO(this->get_logger(), "New prompt cached. Generated UUID: %s", uuid.c_str());
          }
        }
        else
        {
          // no caching, and no uuid. process the prompt directly. since chat mode is not enabled,
          // no need to store the prompt in conversation history. not required to return a uuid either.
          result = prompt_provider_->sendPrompt(input);
          result.buffered = false;

          // convert the result to message and return
          res->response = prompt::toMsg(result);
          res->uuid = "";
          RCLCPP_INFO(this->get_logger(), "New prompt processed.");
        }
      }
      else
      {
        // since uuid is provided, this is a continuation of a previous prompt request. since chat mode is not enabled,
        // the conversation history only contains "user" role prompts and using caching to collect multiple inputs.
        if (req->prompt.use_cache)
        {
          if (req->prompt.flush_cache)
          {
            // flushing the cache, retrieve the conversation history using the provided uuid
            uuid = req->uuid;

            result = prompt_provider_->sendConversation(input, prompt_conversations_[uuid]);
            result.buffered = false;

            // convert the result to message and discontinue the uuid since chat mode disabled
            res->response = prompt::toMsg(result);
            res->uuid = "";

            RCLCPP_INFO(this->get_logger(), "Prompt processed with flushed cache. UUID: %s discontinued.",
                        uuid.c_str());
          }
          else
          {
            // flushing not requested, continue caching, retrieve the conversation history using the provided uuid
            uuid = req->uuid;

            // get the last dialogue from the conversation and append the new prompt to that for caching
            prompt_conversations_[uuid].back().content += " " + req->prompt.prompt;
            result.buffered = true;

            // convert the (buffered-only) result to a message and return the same UUID so the client can continue
            res->response = prompt::toMsg(result);
            res->uuid = uuid;

            RCLCPP_INFO(this->get_logger(),
                        "Prompt cached without flushing in non-chat mode. UUID: %s.",
                        uuid.c_str());
          }
        }
        else
        {
          // no use of cache, chat mode is not enabled, process the prompt directly. No use in uuid either. ignore the
          // uuid provided and process the prompt directly. give a warning.
          RCLCPP_WARN(this->get_logger(), "UUID provided for non-chat prompt with no caching. Ignoring UUID and "
                                          "processing as a generic prompt");

          result = prompt_provider_->sendPrompt(input);
          result.buffered = false;

          // convert the result to message and return
          res->response = prompt::toMsg(result);
          res->uuid = "";
          RCLCPP_INFO(this->get_logger(), "Prompt processed.");
        }
      }
    }

    update_prompt_history(req->prompt, res->response, pre_send_time, this->now());
  }

private:
  // history pub timer callback
  void history_timer()
  {
    // publish the prompt history
    prompt_history_.header.frame_id = frame_id_;
    prompt_history_.header.stamp = this->now();
    prompt_history_pub_->publish(prompt_history_);
  }

  void update_prompt_history(prompt_msgs::msg::Prompt prompt, prompt_msgs::msg::PromptResponse response,
                             rclcpp::Time prompt_time, rclcpp::Time response_time)
  {
    // create the prompt transaction
    prompt_msgs::msg::PromptTransaction prompt_transaction = prompt_msgs::msg::PromptTransaction();

    prompt_transaction.prompt.header = std_msgs::msg::Header();
    prompt_transaction.prompt.header.stamp = prompt_time;
    prompt_transaction.prompt.prompt = prompt;
    prompt_transaction.response.header = std_msgs::msg::Header();
    prompt_transaction.response.header.stamp = response_time;
    prompt_transaction.response.response = response;

    // update the prompt history
    prompt_history_.transactions.push_back(prompt_transaction);

    // remove old transactions
    while (prompt_history_.transactions.size() > transaction_limit_)
    {
      prompt_history_.transactions.erase(prompt_history_.transactions.begin());
    }
  }

private:
  std::string frame_id_;            // frame id
  unsigned int transaction_limit_;  // number of transactions stored in history

  std::string provider_name_;

  // prompt history
  prompt_msgs::msg::PromptHistory prompt_history_;

  // loaders
  pluginlib::ClassLoader<prompt::BaseClass> prompt_provider_loader_;

  // pubs
  rclcpp::Publisher<prompt_msgs::msg::PromptHistory>::SharedPtr prompt_history_pub_;

  // services
  rclcpp::Service<PromptSrv>::SharedPtr prompt_service_;

  // timers
  rclcpp::TimerBase::SharedPtr history_pub_timer_;

  // model families
  std::vector<std::string> model_families_keys_;
  std::map<std::string, std::string> model_families_names_;

  // prompt conversations
  std::map<std::string, std::vector<prompt::PromptDialogue>> prompt_conversations_;
};

}  // namespace prompt
