#pragma once

#include <llm_prompt_provider_plugins/prompt_provider_base.hpp>
#include <memory>
#include <pluginlib/class_loader.hpp>
#include <prompt_msgs/msg/prompt_history.hpp>
#include <prompt_msgs/msg/prompt_transaction.hpp>
#include <prompt_msgs/srv/prompt.hpp>
#include <prompt_schemes/scheme_base.hpp>
#include <rclcpp/rclcpp.hpp>

namespace prompt_bridge
{
class PromptBridge : public rclcpp::Node
{
public:
  PromptBridge(const rclcpp::NodeOptions& options)
    : Node("prompt_bridge", options), loop_hz_(1.0), frame_id_("agent"), transaction_limit_(10)
  {
    // loop rate
    loop_hz_ = this->declare_parameter("loop_rate", 1.0);

    // frame id
    frame_id_ = this->declare_parameter("frame_id", frame_id_);

    // number of transactions stored in history
    transaction_limit_ = this->declare_parameter("cached_transactions", 10);

    // create prompt provider from plugin class loader
    std::string plugin_name = this->declare_parameter("prompt_provider", "prompt_provider::DefaultPromptProvider");

    // create plugin loader
    pluginlib::ClassLoader<prompt_provider::PromptProviderBase> loader("llm_prompt_provider_plugins", "prompt_provider:"
                                                                                                      ":PromptProviderB"
                                                                                                      "ase");

    prompt_provider_ = loader.createUniqueInstance(plugin_name);

    // init provider
    prompt_provider_->init(this->shared_from_this());

    // create prompt scheme from plugin class loader
    std::string scheme = this->declare_parameter("scheme", "prompt_scheme::DefaultScheme");

    // create plugin loader
    pluginlib::ClassLoader<prompt_schemes::SchemeBase> scheme_loader("prompt_schemes", "prompt_schemes::SchemeBase");

    scheme_ = scheme_loader.createUniqueInstance(scheme);

    // init scheme
    scheme_->init(this->shared_from_this());

    // history publisher
    prompt_history_pub_ = this->create_publisher<prompt_msgs::msg::PromptHistory>("history", 1);

    // create prompt service
    prompt_service_ = this->create_service<prompt_msgs::srv::Prompt>(
        "prompt", std::bind(&PromptBridge::prompt_service_cb, this, std::placeholders::_1, std::placeholders::_2));

    // history publisher timer
    history_pub_timer_ = this->create_wall_timer(std::chrono::duration<double>(1.0 / loop_hz_),
                                                 std::bind(&PromptBridge::history_pub_timer_cb, this));
  }

  ~PromptBridge();

  // service callback
  bool prompt_service_cb(const std::shared_ptr<prompt_msgs::srv::Prompt::Request> req,
                         std::shared_ptr<prompt_msgs::srv::Prompt::Response> res)
  {
    // print the prompt message
    RCLCPP_INFO(this->get_logger(), "Prompt: %s", req->prompt.prompt.c_str());

    // pre send time
    auto pre_send_time = this->now();

    // send the prompt message to the prompt provider
    try
    {
      prompt_provider::PromptProviderBase::PromptResponse result =
          prompt_provider_->sendPrompt(prompt_provider_->fromMsg(req->prompt));

      // set the response message
      res->response = prompt_provider_->toMsg(result);
    }
    catch (const std::exception& e)
    {
      RCLCPP_ERROR_STREAM(this->get_logger(), "Prompt provider failed to send prompt: " << e.what());
      return false;
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

    return true;
  }

private:
  // history pub timer callback
  void history_pub_timer_cb()
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

  // number of transactions stored in history
  unsigned int transaction_limit_;

  // prompt history
  prompt_msgs::msg::PromptHistory prompt_history_;

  // prompt provider
  std::shared_ptr<prompt_provider::PromptProviderBase> prompt_provider_;
  // prompt scheme
  std::shared_ptr<prompt_schemes::SchemeBase> scheme_;

  // ROS API
  // pubs
  rclcpp::Publisher<prompt_msgs::msg::PromptHistory>::SharedPtr prompt_history_pub_;
  // services
  rclcpp::Service<prompt_msgs::srv::Prompt>::SharedPtr prompt_service_;
  // timers
  rclcpp::TimerBase::SharedPtr history_pub_timer_;
};

}  // namespace prompt_bridge
