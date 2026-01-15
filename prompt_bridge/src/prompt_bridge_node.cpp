#include <prompt_bridge/prompt_bridge.hpp>

int main(int argc, char* argv[])
{
  // Initialize the ROS 2 C++ client library
  rclcpp::init(argc, argv);

  // Create a shared pointer to the CapabilitiesFabricClient
  auto node = std::make_shared<prompt::PromptBridge>();

  // Initialize the node after construction
  node->initialize();

  // Use a MultiThreadedExecutor to spin the node
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();

  return 0;
}