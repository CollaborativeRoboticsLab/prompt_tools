// test_prompt.cpp
#include "prompt_bridge/test_prompt.hpp"
#include <rclcpp/executors/multi_threaded_executor.hpp>


int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto prompt_node = std::make_shared<prompt_test::TestPromptNode>();
  auto embedding_node = std::make_shared<prompt_test::TestEmbeddingNode>();
  auto tokenizer_node = std::make_shared<prompt_test::TestTokenizerNode>();
  rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), 4);
  executor.add_node(prompt_node);
  executor.add_node(embedding_node);
  executor.add_node(tokenizer_node);
  executor.spin();
  rclcpp::shutdown();
  return 0;
}
