#pragma once

#include <uuid/uuid.h>

#include <functional>
#include <memory>
#include <pluginlib/class_loader.hpp>
#include <prompt_base/base_class.hpp>
#include <prompt_base/utils/conversions.hpp>
#include <prompt_base/utils/exceptions.hpp>
#include <prompt_base/utils/model_family_options.hpp>
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

    model_families_names_ = prompt::load_model_families(shared_from_this());

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

    // pre send time
    auto pre_send_time = this->now();

    prompt::PromptRequest input = prompt::fromMsg(req->prompt);
    prompt::PromptResponse result;

    // check if the model family exists
    const std::string& model_family = input.model_family;
    if (model_families_names_.find(model_family) != model_families_names_.end())
    {
      // check if the prompt type plugin exists
      const std::string prompt_type = input.use_chat_mode ? "chat" : "single";

      if (model_families_names_[model_family].find(prompt_type) != model_families_names_[model_family].end())
      {
        prompt_provider_ =
            prompt_provider_loader_.createSharedInstance(model_families_names_[model_family][prompt_type]);
        prompt_provider_->initialize(shared_from_this());

        result = prompt_provider_->sendPrompt(input);

        // set the response message
        res->response = prompt::toMsg(result);

        update_prompt_history(req->prompt, res->response, pre_send_time, this->now());
      }
      else
      {
        RCLCPP_ERROR(this->get_logger(), "Prompt type plugin not found for model family");
        throw prompt::PromptException("Prompt type plugin not found for model family");
      }
    }
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

  // prompt provider
  std::shared_ptr<prompt::BaseClass> prompt_provider_;

  // pubs
  rclcpp::Publisher<prompt_msgs::msg::PromptHistory>::SharedPtr prompt_history_pub_;

  // services
  rclcpp::Service<PromptSrv>::SharedPtr prompt_service_;

  // timers
  rclcpp::TimerBase::SharedPtr history_pub_timer_;

  // model families
  std::map<std::string, std::map<std::string, std::string>> model_families_names_;
};

}  // namespace prompt
