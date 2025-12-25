#pragma once

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
    client_ = this->create_client<prompt_msgs::srv::Prompt>("prompt/prompt");
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

    // 1. Stateless prompt
    send_prompt("What is the capital of France?", false, false, false, "openai", "");
    // 2. Chat mode
    send_prompt("Hello, who are you?", false, false, true, "openai", "");

    // 3. Caching (get uuid for flush)
    std::string cache_uuid = send_prompt("First part of a multi-input.", true, false, false, "openai", "");
    // 4. Flush cache (use uuid from previous response)
    if (!cache_uuid.empty())
    {
      send_prompt("", true, true, false, "openai", cache_uuid);
    }
    // 5. Model selection and options
    send_prompt("Test with model option.", false, false, false, "ollama", "");
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

    auto future = client_->async_send_request(
        req, [&cv, &mtx, &response_uuid, &done, this](rclcpp::Client<prompt_msgs::srv::Prompt>::SharedFuture future) {
          auto res = future.get();
          RCLCPP_INFO(this->get_logger(), "Response: %s | UUID: %s | Success: %d", res->response.response.c_str(),
                      res->uuid.c_str(), res->response.success);

          std::lock_guard<std::mutex> lock(mtx);
          response_uuid = res->uuid;
          done = true;

          cv.notify_one();
        });

    // Block until response is received
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&done] { return done; });
    return response_uuid;
  }

  // handle_response is now inlined in send_prompt

  rclcpp::Client<prompt_msgs::srv::Prompt>::SharedPtr client_;
  rclcpp::TimerBase::SharedPtr timer_;
};
}  // namespace prompt_test