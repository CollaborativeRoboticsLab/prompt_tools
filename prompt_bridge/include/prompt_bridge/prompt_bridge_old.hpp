#pragma once

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
    : Node("prompt_bridge", options)
    , prompt_provider_loader_("prompt_bridge", "prompt::BaseClass")
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
    this->declare_parameter("use_prompt_provider", false);
    this->declare_parameter("frame_id", "agent");
    this->declare_parameter("cached_transactions", 10);

    use_prompt_provider_ = this->get_parameter("use_prompt_provider").as_bool();
    frame_id_ = this->get_parameter("frame_id").as_string();
    transaction_limit_ = this->get_parameter("cached_transactions").as_int();

    /*************************************************************************
     * prompt provider plugin class loader and provider pointer
     ************************************************************************/

    if (use_prompt_provider_)
    {
      RCLCPP_INFO(this->get_logger(), "Prompt provider is enabled");

      this->declare_parameter("prompt_provider", "prompt::DefaultPromptProvider");
      provider_name_ = this->get_parameter("prompt_provider").as_string();

      if (provider_name_.empty())
      {
        RCLCPP_ERROR(this->get_logger(), "Prompt provider name is empty, use avalid provider name");
        throw prompt::PromptException("Prompt provider name is empty, use avalid provider name");
      }

      RCLCPP_INFO(this->get_logger(), "Loading prompt provider plugin: '%s'", provider_name_.c_str());

      prompt_provider_ = prompt_provider_loader_.createSharedInstance(provider_name_);
      prompt_provider_->initialize(shared_from_this());
      RCLCPP_INFO(this->get_logger(), "Prompt provider '%s' initialized", provider_name_.c_str());
    }
    else
    {
      RCLCPP_INFO(this->get_logger(), "Not using prompt provider");
    }

    /*************************************************************************
     * prompt service and history ros interfaces
     ************************************************************************/

    // history publisher
    prompt_history_pub_ = this->create_publisher<prompt_msgs::msg::PromptHistory>("/history/prompt_bridge", 1);

    // history publisher timer
    history_pub_timer_ =
        this->create_wall_timer(std::chrono::duration<double>(1.0), std::bind(&PromptBridge::history_timer, this));

    // prompt service interface
    prompt_service_ = this->create_service<PromptSrv>(
        "prompt_bridge/prompt",
        std::bind(&PromptBridge::prompt_service_cb, this, std::placeholders::_1, std::placeholders::_2));

    RCLCPP_INFO(this->get_logger(), "Prompt service created at 'prompt_bridge/prompt'");

    // prompt action server
    // TODO: add support for streaming (prompt action server)
    // this->prompt_action_server_ = rclcpp_action::create_server<prompt_msgs::action::Prompt>(
    //     this, "~/prompt", std::bind(&PromptBridge::handle_goal, this, std::placeholders::_1,
    //     std::placeholders::_2), std::bind(&PromptBridge::handle_cancel, this, std::placeholders::_1),
    //     std::bind(&PromptBridge::handle_accepted, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "PromptBridge initialized");
  }

  ~PromptBridge() = default;

  /**
   * @brief prompt service callback
   *
   * This function is called when a prompt service request is received. It processes the prompt request
   * using the prompt provider, and sends the response back to the client.
   *
   * @param req the prompt service request. contains a prompt message, which is processed by the prompt provider
   * @param res the prompt service response. contains the processed prompt response.
   *
   * @throws prompt::PromptException if the prompt provider fails to process the prompt
   */
  void prompt_service_cb(const std::shared_ptr<PromptSrv::Request> req, std::shared_ptr<PromptSrv::Response> res)
  {
    // pre send time
    auto pre_send_time = this->now();

    prompt::PromptRequest input = prompt::fromMsg(req->prompt);
    prompt::PromptResponse result;

    try
    {
      result = prompt_provider_->sendPrompt(input);
    }
    catch (const prompt::PromptException& e)
    {
      RCLCPP_ERROR_STREAM(this->get_logger(), "Prompt scheme failed to process prompt: " << e.what());
      throw prompt::PromptException("Prompt scheme failed to send prompt");
    }

    // set the response message
    res->response = prompt::toMsg(result);

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
  // ros params
  bool use_prompt_provider_;

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

  // actions
  // rclcpp_action::Client<prompt_msgs::action::Prompt>::SharedPtr prompt_action_client_;
  // rclcpp_action::Server<PromptPlan>::SharedPtr plan_action_server_;

  // timers
  rclcpp::TimerBase::SharedPtr history_pub_timer_;
};

}  // namespace prompt
