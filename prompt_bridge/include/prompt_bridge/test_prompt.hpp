#pragma once

#include <rmw/qos_profiles.h>

#include <chrono>
#include <memory>
#include <prompt_msgs/msg/embed.hpp>
#include <prompt_msgs/msg/embed_response.hpp>
#include <prompt_msgs/msg/prompt.hpp>
#include <prompt_msgs/msg/prompt_response.hpp>
#include <prompt_msgs/srv/embedding.hpp>
#include <prompt_msgs/srv/prompt.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>
#include <vector>
using namespace std::chrono_literals;

namespace prompt_test
{
class TestPromptNode : public rclcpp::Node
{
public:
  TestPromptNode() : Node("test_prompt_node")
  {
    client_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
    client_ =
        this->create_client<prompt_msgs::srv::Prompt>("prompt/prompt", rmw_qos_profile_services_default, client_group_);
    timer_ = this->create_wall_timer(2s, std::bind(&TestPromptNode::run_tests, this));
  }

private:
  void run_tests()
  {
    if (!client_->wait_for_service(1s))
    {
      RCLCPP_WARN(this->get_logger(), "Prompt service not available yet.");
      return;
    }
    timer_->cancel();
    RCLCPP_INFO(this->get_logger(), "Running prompt_bridge feature tests...");
    test_step_ = 1;
    last_uuid_.clear();
    send_next_prompt();
  }

  void send_next_prompt()
  {
    auto req = std::make_shared<prompt_msgs::srv::Prompt::Request>();
    switch (test_step_)
    {
      case 1:
        req->uuid = "";
        req->prompt.prompt = "What is the capital of France?";
        req->prompt.use_cache = false;
        req->prompt.flush_cache = false;
        req->prompt.use_chat_mode = false;
        req->prompt.model_family = "openai";
        break;
      case 2:
        req->uuid = "";
        req->prompt.prompt = "Hello, who are you?";
        req->prompt.use_cache = false;
        req->prompt.flush_cache = false;
        req->prompt.use_chat_mode = true;
        req->prompt.model_family = "openai";
        break;
      case 3:
        req->uuid = "";
        req->prompt.prompt = "Chat cached part one.";
        req->prompt.use_cache = true;
        req->prompt.flush_cache = false;
        req->prompt.use_chat_mode = true;
        req->prompt.model_family = "openai";
        break;
      case 4:
        req->uuid = last_uuid_;
        req->prompt.prompt = "Chat cached part two.";
        req->prompt.use_cache = true;
        req->prompt.flush_cache = true;
        req->prompt.use_chat_mode = true;
        req->prompt.model_family = "openai";
        break;
      case 5:
        req->uuid = "";
        req->prompt.prompt = "First part of a multi-input.";
        req->prompt.use_cache = true;
        req->prompt.flush_cache = false;
        req->prompt.use_chat_mode = false;
        req->prompt.model_family = "openai";
        break;
      case 6:
        req->uuid = last_uuid_;
        req->prompt.prompt = "Flushing cached non-chat prompt.";
        req->prompt.use_cache = true;
        req->prompt.flush_cache = true;
        req->prompt.use_chat_mode = false;
        req->prompt.model_family = "openai";
        break;
      default:
        return;
    }
    RCLCPP_INFO(this->get_logger(), "Sending prompt: %s", req->prompt.prompt.c_str());
    client_->async_send_request(req, std::bind(&TestPromptNode::handle_response, this, std::placeholders::_1));
  }

  void handle_response(rclcpp::Client<prompt_msgs::srv::Prompt>::SharedFuture future)
  {
    auto res = future.get();
    RCLCPP_INFO(this->get_logger(), "Response: %s | UUID: %s | Success: %d", res->response.response.c_str(),
                res->uuid.c_str(), res->response.success);
    if (test_step_ == 3 || test_step_ == 5)
    {
      last_uuid_ = res->uuid;
    }
    ++test_step_;
    if (test_step_ <= 6)
    {
      send_next_prompt();
    }
  }

  rclcpp::Client<prompt_msgs::srv::Prompt>::SharedPtr client_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::CallbackGroup::SharedPtr client_group_;
  int test_step_;
  std::string last_uuid_;
};

class TestEmbeddingNode : public rclcpp::Node
{
public:
  TestEmbeddingNode() : Node("test_embedding_node")
  {
    client_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
    client_ = this->create_client<prompt_msgs::srv::Embedding>("prompt/embedding", rmw_qos_profile_services_default,
                                                               client_group_);
    timer_ = this->create_wall_timer(std::chrono::seconds(2), std::bind(&TestEmbeddingNode::run_test, this));
  }

private:
  void run_test()
  {
    if (!client_->wait_for_service(std::chrono::seconds(1)))
    {
      RCLCPP_WARN(this->get_logger(), "Embedding service not available yet.");
      return;
    }
    timer_->cancel();
    RCLCPP_INFO(this->get_logger(), "Running embedding interface test...");

    auto req = std::make_shared<prompt_msgs::srv::Embedding::Request>();
    req->input.text = "What is the capital of Germany?";
    req->input.model_family = "openai";

    // Optionally set options/format here

    client_->async_send_request(req, [this](rclcpp::Client<prompt_msgs::srv::Embedding>::SharedFuture future) {
      auto res = future.get();
      RCLCPP_INFO(this->get_logger(),
                  "Embedding response: success=%d, float_embedding.size=%zu, base64_embedding='%s', error='%s'",
                  res->output.success, res->output.float_embedding.size(), res->output.base64_embedding.c_str(),
                  res->output.error.c_str());
    });
  }

  rclcpp::Client<prompt_msgs::srv::Embedding>::SharedPtr client_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::CallbackGroup::SharedPtr client_group_;
};
}  // namespace prompt_test