// test_prompt.cpp
#include "prompt_bridge/test_prompt.hpp"

#include <rclcpp/executors/multi_threaded_executor.hpp>

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<prompt_test::TestPromptNode>();
  rclcpp::executors::MultiThreadedExecutor executor(rclcpp::ExecutorOptions(), 2);
  executor.add_node(node);
  executor.spin();
  rclcpp::shutdown();
  return 0;
}
