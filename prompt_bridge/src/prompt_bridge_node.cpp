#include <ros/ros.h>
#include <prompt_bridge/prompt_bridge.hpp>

// main function
int main(int argc, char** argv)
{
  // initialize the node
  ros::init(argc, argv, "prompt_bridge_server");

  // create the PromptBridge object
  prompt_bridge::PromptBridge prompt_bridge;

  // spin the node
  ros::spin();

  return 0;
}
