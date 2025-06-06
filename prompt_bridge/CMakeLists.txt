cmake_minimum_required(VERSION 3.10.2)
project(prompt_bridge)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(pluginlib REQUIRED)
find_package(prompt_msgs REQUIRED)
find_package(prompt_base REQUIRED)
find_package(backward_ros REQUIRED)

include_directories(
  include
)

# component library
add_library(${PROJECT_NAME} SHARED
  src/prompt_bridge.cpp
)

ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  rclcpp_components
  rclcpp_action
  pluginlib
  prompt_msgs
  prompt_base
)

# register component
rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "prompt_bridge::PromptBridge"
  EXECUTABLE prompt_bridge_node
)

ament_export_targets(prompt_bridge_node)

install(TARGETS ${PROJECT_NAME}
  EXPORT prompt_bridge_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
