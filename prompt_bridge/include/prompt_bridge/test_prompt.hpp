#pragma once

#include <rmw/qos_profiles.h>

#include <chrono>
#include <memory>
#include <prompt_msgs/msg/embed.hpp>
#include <prompt_msgs/msg/embed_response.hpp>
#include <prompt_msgs/msg/model_option.hpp>
#include <prompt_msgs/msg/prompt.hpp>
#include <prompt_msgs/msg/prompt_response.hpp>
#include <prompt_msgs/srv/embedding.hpp>
#include <prompt_msgs/srv/prompt.hpp>
#include <prompt_msgs/srv/tokenize.hpp>
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
    RCLCPP_INFO(this->get_logger(), "Running embedding interface tests (FLOAT + BASE64)...");

    test_step_ = 1;
    send_next_embedding();
  }

  void send_next_embedding()
  {
    auto req = std::make_shared<prompt_msgs::srv::Embedding::Request>();
    req->input.text = "Hello, how are you?";
    req->input.model_family = "openai";

    if (test_step_ == 1)
    {
      prompt_msgs::msg::ModelOption option1;
      option1.key = "encoding_format";
      option1.value = "float";
      option1.type = prompt_msgs::msg::ModelOption::STRING_TYPE;

      req->input.options.push_back(option1);
      RCLCPP_INFO(this->get_logger(), "Requesting FLOAT embedding format");
    }
    else if (test_step_ == 2)
    {
      prompt_msgs::msg::ModelOption option2;
      option2.key = "encoding_format";
      option2.value = "base64";
      option2.type = prompt_msgs::msg::ModelOption::STRING_TYPE;

      req->input.options.push_back(option2);
      RCLCPP_INFO(this->get_logger(), "Requesting BASE64 embedding format");
    }
    else
    {
      return;
    }

    client_->async_send_request(req,
                                std::bind(&TestEmbeddingNode::handle_embedding_response, this, std::placeholders::_1));
  }

  void handle_embedding_response(rclcpp::Client<prompt_msgs::srv::Embedding>::SharedFuture future)
  {
    auto res = future.get();
    const auto& out = res->output;
    size_t emb_count = out.embeddings.size();
    int first_index = (emb_count > 0) ? out.embeddings[0].index : -1;
    bool first_is_float = (emb_count > 0) ? out.embeddings[0].is_float : false;
    size_t first_float_size = (emb_count > 0) ? out.embeddings[0].float_embedding.size() : 0;
    size_t first_b64_len = (emb_count > 0) ? out.embeddings[0].base64_embedding.size() : 0;

    RCLCPP_INFO(this->get_logger(),
                "Embedding response: success=%d, embeddings=%zu, first.index=%d, first.is_float=%d, float.size=%zu, "
                "base64.len=%zu, model='%s', prompt_tokens=%d, total_tokens=%d, error='%s'",
                out.success, emb_count, first_index, first_is_float, first_float_size, first_b64_len, out.model.c_str(),
                out.prompt_tokens, out.total_tokens, out.error.c_str());

    if (!out.success)
    {
      RCLCPP_ERROR(this->get_logger(), "Embedding request failed: %s", out.error.c_str());
    }

    if (test_step_ == 1)
    {
      if (emb_count == 0)
      {
        RCLCPP_ERROR(this->get_logger(), "Expected at least one embedding in response.");
      }
      else if (!out.embeddings[0].is_float)
      {
        RCLCPP_ERROR(this->get_logger(), "Expected FLOAT format in response.");
      }
      if (out.embeddings[0].float_embedding.empty())
      {
        RCLCPP_ERROR(this->get_logger(), "FLOAT embedding array is empty.");
      }
      if (!out.embeddings[0].base64_embedding.empty())
      {
        RCLCPP_WARN(this->get_logger(), "Base64 embedding should be empty when FLOAT requested.");
      }
    }
    else if (test_step_ == 2)
    {
      if (emb_count == 0)
      {
        RCLCPP_ERROR(this->get_logger(), "Expected at least one embedding in response.");
      }
      else if (out.embeddings[0].is_float)
      {
        RCLCPP_ERROR(this->get_logger(), "Expected BASE64 format in response.");
      }
      if (out.embeddings[0].base64_embedding.empty())
      {
        RCLCPP_ERROR(this->get_logger(), "BASE64 embedding string is empty.");
      }
      if (!out.embeddings[0].float_embedding.empty())
      {
        RCLCPP_WARN(this->get_logger(), "Float embedding should be empty when BASE64 requested.");
      }
    }

    ++test_step_;
    if (test_step_ <= 2)
    {
      send_next_embedding();
    }
  }

  rclcpp::Client<prompt_msgs::srv::Embedding>::SharedPtr client_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::CallbackGroup::SharedPtr client_group_;
  int test_step_ = 0;
};

class TestTokenizerNode : public rclcpp::Node
{
public:
  TestTokenizerNode() : Node("test_tokenizer_node")
  {
    client_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
    client_ = this->create_client<prompt_msgs::srv::Tokenize>("prompt/tokenizer", rmw_qos_profile_services_default,
                                                              client_group_);
    timer_ = this->create_wall_timer(std::chrono::seconds(2), std::bind(&TestTokenizerNode::run_test, this));
  }

private:
  void run_test()
  {
    if (!client_->wait_for_service(std::chrono::seconds(1)))
    {
      RCLCPP_WARN(this->get_logger(), "Tokenizer service not available yet.");
      return;
    }
    timer_->cancel();
    RCLCPP_INFO(this->get_logger(), "Running tokenizer interface tests (ENCODE + DECODE)...");
    test_step_ = 1;
    send_next_tokenize();
  }

  void send_next_tokenize()
  {
    auto req = std::make_shared<prompt_msgs::srv::Tokenize::Request>();
    req->input.model_family = "openai";
    prompt_msgs::msg::ModelOption option;
    option.key = "model";
    option.value = "O200K_BASE";
    option.type = prompt_msgs::msg::ModelOption::STRING_TYPE;
    req->input.options.push_back(option);

    if (test_step_ == 1)
    {
      req->input.text = "Encode this text to tokens.";
      req->input.encode = true;
      RCLCPP_INFO(this->get_logger(), "Requesting tokenization (encode) for: %s", req->input.text.c_str());
    }
    else if (test_step_ == 2)
    {
      req->input.tokens = last_tokens_;
      req->input.encode = false;
      RCLCPP_INFO(this->get_logger(), "Requesting detokenization (decode) for %zu tokens.", last_tokens_.size());
    }
    else
    {
      return;
    }

    client_->async_send_request(req,
                                std::bind(&TestTokenizerNode::handle_tokenize_response, this, std::placeholders::_1));
  }

  void handle_tokenize_response(rclcpp::Client<prompt_msgs::srv::Tokenize>::SharedFuture future)
  {
    auto res = future.get();
    const auto& out = res->output;
    if (!out.success)
    {
      RCLCPP_ERROR(this->get_logger(), "Tokenizer request failed: %s", out.error.c_str());
      return;
    }
    if (test_step_ == 1)
    {
      last_tokens_ = out.tokens;
      std::string first_token_str = out.tokens.empty() ? "" : std::to_string(out.tokens[0]);
      RCLCPP_INFO(this->get_logger(), "Encoded tokens: [%s] (count=%zu)",
                  first_token_str.c_str(), out.tokens.size());
      for (size_t i = 1; i < out.tokens.size(); ++i)
        RCLCPP_INFO(this->get_logger(), "  token[%zu]=%d", i, out.tokens[i]);
      ++test_step_;
      send_next_tokenize();
    }
    else if (test_step_ == 2)
    {
      RCLCPP_INFO(this->get_logger(), "Decoded text: '%s'", out.text.c_str());
    }
  }

  rclcpp::Client<prompt_msgs::srv::Tokenize>::SharedPtr client_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::CallbackGroup::SharedPtr client_group_;
  int test_step_ = 0;
  std::vector<int32_t> last_tokens_;
};

}  // namespace prompt_test