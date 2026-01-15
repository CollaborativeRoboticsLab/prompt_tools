#pragma once

#include <rmw/qos_profiles.h>

#include <chrono>
#include <memory>
#include <prompt_msgs/msg/prompt.hpp>
#include <prompt_msgs/msg/prompt_response.hpp>
#include <prompt_msgs/srv/prompt.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>

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

    // 1. Stateless prompt (no chat, no cache)
    std::string stateless_uuid =
        send_prompt("What is the capital of France?", false, false, false, "openai", "");
    RCLCPP_INFO(this->get_logger(), "Stateless prompt UUID (expected empty): '%s'", stateless_uuid.c_str());

    // 2. Chat mode, no cache – should return a non-empty UUID
    std::string chat_uuid = send_prompt("Hello, who are you?", false, false, true, "openai", "");
    RCLCPP_INFO(this->get_logger(), "Chat prompt UUID (expected non-empty): '%s'", chat_uuid.c_str());

    // 3. Chat mode with cache: first call buffers only, returns a UUID
    std::string chat_cache_uuid =
        send_prompt("Chat cached part one.", true, false, true, "openai", "");
    RCLCPP_INFO(this->get_logger(), "Chat cache UUID (expected non-empty): '%s'", chat_cache_uuid.c_str());

    // 4. Chat mode cache flush: use same UUID, expect a real response
    if (!chat_cache_uuid.empty())
    {
      std::string chat_flush_uuid =
          send_prompt("Chat cached part two.", true, true, true, "openai", chat_cache_uuid);
      RCLCPP_INFO(this->get_logger(), "Chat flush UUID (should match cache UUID): '%s'",
                  chat_flush_uuid.c_str());
    }

    // 5. Non-chat caching (get uuid for flush)
    std::string cache_uuid = send_prompt("First part of a multi-input.", true, false, false, "openai", "");
    RCLCPP_INFO(this->get_logger(), "Non-chat cache UUID (expected non-empty): '%s'", cache_uuid.c_str());

    // 6. Non-chat flush cache (use uuid from previous response)
    if (!cache_uuid.empty())
    {
      send_prompt("Flushing cached non-chat prompt.", true, true, false, "openai", cache_uuid);
    }
  }

  std::string send_prompt(const std::string& prompt_text, bool use_cache, bool flush_cache, bool use_chat_mode,
                          const std::string& model_family, const std::string& uuid)
  {
    auto req = std::make_shared<prompt_msgs::srv::Prompt::Request>();
    req->uuid = uuid;
    req->prompt.prompt = prompt_text;
    req->prompt.use_cache = use_cache;
    req->prompt.flush_cache = flush_cache;
    req->prompt.use_chat_mode = use_chat_mode;
    req->prompt.model_family = model_family;
    // Optionally add model options here
    RCLCPP_INFO(this->get_logger(), "Sending prompt: %s", prompt_text.c_str());

    std::mutex mtx;
    std::condition_variable cv;
    std::string response_uuid;

    bool done = false;
    std::unique_lock<std::mutex> lock(mtx);

    auto future = client_->async_send_request(
        req, [this, &cv, &mtx, &response_uuid, &done](rclcpp::Client<prompt_msgs::srv::Prompt>::SharedFuture future) {
          auto res = future.get();

          std::lock_guard<std::mutex> guard(mtx);
          RCLCPP_INFO(this->get_logger(), "Response: %s | UUID: %s | Success: %d", res->response.response.c_str(),
                      res->uuid.c_str(), res->response.success);
          response_uuid = res->uuid;
          done = true;

          cv.notify_all();
        });

    // Block until response is received
    cv.wait(lock, [&done] { return done; });
    return response_uuid;
  }

  // handle_response is now inlined in send_prompt

  rclcpp::Client<prompt_msgs::srv::Prompt>::SharedPtr client_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::CallbackGroup::SharedPtr client_group_;
};
}  // namespace prompt_test