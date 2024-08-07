#pragma once

#include <memory>

#include <rclcpp/rclcpp.hpp>
#include <prompt_msgs/msg/prompt_history.hpp>
#include <prompt_msgs/msg/prompt_transaction.hpp>
#include <prompt_msgs/srv/prompt.hpp>
#include <llm_prompt_provider_plugins/prompt_provider.hpp>

namespace prompt_bridge
{
class PromptBridge : public rclcpp::Node
{
public:
  PromptBridge() : Node("prompt_bridge"), loop_hz_(1.0), frame_id_("agent"), transaction_limit_(10)
  {
    // loop rate
    loop_hz_ = this->declare_parameter("loop_rate", 1.0);

    // frame id
    frame_id_ = this->declare_parameter("frame_id", frame_id_);

    // number of transactions stored in history
    transaction_limit_ = this->declare_parameter("cached_transactions", 10);

    // create prompt provider from plugin class
    // prompt_provider_ = std::make_shared<prompt_provider::PromptProvider>(shared_from_this());

    // history publisher
    prompt_history_pub_ = this->create_publisher<prompt_msgs::msg::PromptHistory>("history", 1);

    // create prompt service
    prompt_service_ = this->create_service<prompt_msgs::srv::Prompt>(
        "prompt", std::bind(&PromptBridge::prompt_service_cb, this, std::placeholders::_1, std::placeholders::_2));

    // prompt publisher timer
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
      prompt_provider::PromptProvider::PromptResponse result =
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
  int transaction_limit_;

  // prompt history
  prompt_msgs::msg::PromptHistory prompt_history_;

  // prompt provider
  std::shared_ptr<prompt_provider::PromptProvider> prompt_provider_;

  // ROS API
  // subs
  // pubs
  rclcpp::Publisher<prompt_msgs::msg::PromptHistory>::SharedPtr prompt_history_pub_;
  // services
  rclcpp::Service<prompt_msgs::srv::Prompt>::SharedPtr prompt_service_;
  // timers
  rclcpp::TimerBase::SharedPtr history_pub_timer_;
};

}  // namespace prompt_bridge
