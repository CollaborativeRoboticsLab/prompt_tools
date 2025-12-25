// test_prompt.cpp
#include "prompt_bridge/test_prompt.hpp"

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<prompt_test::TestPromptNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
