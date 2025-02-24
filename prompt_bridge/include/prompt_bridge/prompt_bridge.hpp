#pragma once

#include <functional>
#include <memory>
#include <pluginlib/class_loader.hpp>
#include <prompt_msgs/msg/prompt_history.hpp>
#include <prompt_msgs/msg/prompt_transaction.hpp>
#include <prompt_msgs/srv/prompt.hpp>
#include <prompt_provider/prompt_provider_base.hpp>
#include <prompt_scheme/scheme_base.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <thread>

namespace prompt_bridge
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
  PromptBridge(const rclcpp::NodeOptions& options = rclcpp::NodeOptions())
    : Node("prompt_bridge", options)
    , loop_hz_(1.0)
    , frame_id_("agent")
    , use_scheme_(false)
    , transaction_limit_(10)
    // , prompt_history_()
    , prompt_provider_loader_("prompt_provider_plugins", "prompt_provider::PromptProviderBase")
    , prompt_scheme_loader_("prompt_scheme", "prompt_scheme::SchemeBase")
  {
    // loop rate
    loop_hz_ = this->declare_parameter("loop_rate", loop_hz_);

    // frame id
    frame_id_ = this->declare_parameter("frame_id", frame_id_);

    // number of transactions stored in history
    transaction_limit_ = this->declare_parameter("cached_transactions", 10);

    // number of transactions stored in history
    use_scheme_ = this->declare_parameter("use_scheme", false);

    /*************************************************************************
     * prompt provider plugin class loader and provider pointer
     ************************************************************************/

    provider_name_ = this->declare_parameter("prompt_provider_plugin", "prompt_provider::DefaultPromptProvider");

    RCLCPP_INFO(this->get_logger(), "Loading prompt provider plugin: '%s'", provider_name_.c_str());

    prompt_provider_ = prompt_provider_loader_.createSharedInstance(provider_name_);

    // init provider and get a shared pointer to the node parameters interface
    prompt_provider_->init(this->get_node_parameters_interface(), this->get_node_logging_interface());

    /*************************************************************************
     * prompt scheme plugin class loader and scheme pointer
     ************************************************************************/

    scheme_name_ = this->declare_parameter("prompt_scheme_plugin", "prompt_scheme::DefaultScheme");

    RCLCPP_INFO(this->get_logger(), "Loading prompt provider plugin: '%s'", scheme_name_.c_str());

    prompt_scheme_ = prompt_scheme_loader_.createSharedInstance(scheme_name_);

    // init provider and get a shared pointer to the node parameters interface
    prompt_scheme_->init(this->get_node_parameters_interface(), this->get_node_logging_interface());

    // set the prompt provider
    prompt_scheme_->set_prompt_provider(prompt_provider_);

    /*************************************************************************
     * prompt service and history ros interfaces
     ************************************************************************/

    // history publisher
    prompt_history_pub_ = this->create_publisher<prompt_msgs::msg::PromptHistory>("~/history/bridge", 1);

    // prompt service interface
    prompt_service_ = this->create_service<PromptSrv>(
        "~/prompt", std::bind(&PromptBridge::prompt_service_cb, this, std::placeholders::_1, std::placeholders::_2));

    // prompt action server
    // TODO: add support for streaming (prompt action server)
    // this->prompt_action_server_ = rclcpp_action::create_server<prompt_msgs::action::Prompt>(
    //     this, "~/prompt", std::bind(&PromptBridge::handle_goal, this, std::placeholders::_1, std::placeholders::_2),
    //     std::bind(&PromptBridge::handle_cancel, this, std::placeholders::_1),
    //     std::bind(&PromptBridge::handle_accepted, this, std::placeholders::_1));

    // history publisher timer
    history_pub_timer_ =
        this->create_wall_timer(std::chrono::duration<double>(1.0), std::bind(&PromptBridge::history_timer, this));

    RCLCPP_INFO(this->get_logger(), "PromptBridge initialized");
  }

  ~PromptBridge() = default;

  // service callback
  /**
   * @brief prompt service callback
   *
   * send and receive prompt messages to the prompt provider
   * this service is scheme-less and just transacts a prompt
   *
   * @param req
   * @param res
   */
  void prompt_service_cb(const std::shared_ptr<PromptSrv::Request> req, std::shared_ptr<PromptSrv::Response> res)
  {
    // print the prompt message
    // RCLCPP_DEBUG(this->get_logger(), "Prompt: %s", req->prompt.prompt.c_str());

    // pre send time
    auto pre_send_time = this->now();

    // send the prompt message to the prompt provider
    if (use_scheme_)
    {
    }
    else
    {
      try
      {
        prompt::PromptResponse result = prompt_provider_->sendPrompt(prompt_provider_->fromMsg(req->prompt));

        // set the response message
        res->response = prompt_provider_->toMsg(result);
      }
      catch (const prompt::PromptProviderException& e)
      {
        RCLCPP_ERROR_STREAM(this->get_logger(), "Prompt provider failed to send prompt: " << e.what());

        // throw exception
        throw std::runtime_error("Prompt provider failed to send prompt");
      }
    }

    // post send time
    auto post_send_time = this->now();

    // create the prompt transaction
    prompt_msgs::msg::PromptTransaction prompt_transaction = prompt_msgs::msg::PromptTransaction();

    prompt_transaction.prompt.header = std_msgs::msg::Header();
    prompt_transaction.prompt.header.stamp = pre_send_time;
    prompt_transaction.prompt.prompt = req->prompt;
    prompt_transaction.response.header = std_msgs::msg::Header();
    prompt_transaction.response.header.stamp = post_send_time;
    prompt_transaction.response.response = res->response;

    // update the prompt history
    prompt_history_.transactions.push_back(prompt_transaction);

    // remove old transactions
    while (prompt_history_.transactions.size() > transaction_limit_)
    {
      prompt_history_.transactions.erase(prompt_history_.transactions.begin());
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

private:
  // ros params
  double loop_hz_;        // loop rate
  std::string frame_id_;  // frame id
  bool use_scheme_;       // use scheme or not
  std::string provider_name_;
  std::string scheme_name_;

  // number of transactions stored in history
  unsigned int transaction_limit_;

  // prompt history
  prompt_msgs::msg::PromptHistory prompt_history_;

  // loaders
  pluginlib::ClassLoader<prompt_provider::PromptProviderBase> prompt_provider_loader_;
  pluginlib::ClassLoader<prompt_scheme::SchemeBase> prompt_scheme_loader_;

  // prompt scheme
  std::shared_ptr<prompt_scheme::SchemeBase> prompt_scheme_;

  // prompt provider
  std::shared_ptr<prompt_provider::PromptProviderBase> prompt_provider_;

  // ROS API
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

}  // namespace prompt_bridge
