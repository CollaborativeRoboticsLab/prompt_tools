#include <rclcpp/rclcpp.hpp>
#include <prompt_bridge/prompt_bridge.hpp>

// main function
int main(int argc, char** argv)
{
  // initialize the node
  // ros::init(argc, argv, "prompt_bridge_server");
  rclcpp::init(argc, argv);

  // create the node
  auto node = std::make_shared<prompt_bridge::PromptBridge>();

  // spin the node
  rclcpp::spin(node);

  // shutdown the node
  rclcpp::shutdown();

  return 0;
}
