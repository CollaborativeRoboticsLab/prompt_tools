cmake_minimum_required(VERSION 3.10.2)
project(prompt_bridge)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(prompt_msgs REQUIRED)
find_package(llm_prompt_provider_plugins REQUIRED)

include_directories(
  include
  ${prompt_msgs_INCLUDE_DIRS}
  ${llm_prompt_provider_plugins_INCLUDE_DIRS}
)

# bridge server node
add_executable(prompt_bridge_server src/prompt_bridge_node.cpp)

ament_target_dependencies(prompt_bridge_server
  rclcpp
  prompt_msgs
  llm_prompt_provider_plugins
)

install(TARGETS prompt_bridge_server
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
