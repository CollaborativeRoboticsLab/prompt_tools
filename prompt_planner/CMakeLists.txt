cmake_minimum_required(VERSION 3.8)
project(prompt_planner)

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
find_package(prompt_provider REQUIRED)
find_package(prompt_scheme REQUIRED)
find_package(prompt_utils REQUIRED)
find_package(backward_ros REQUIRED)

include_directories(
  include
)

# component library
add_library(${PROJECT_NAME} SHARED
  src/prompt_planner.cpp
)

ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  rclcpp_components
  rclcpp_action
  pluginlib
  prompt_msgs
  prompt_provider
  prompt_scheme
  prompt_utils
)

# register component
rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN "prompt_planner::PromptPlanner"
  EXECUTABLE prompt_planner_node
)

ament_export_targets(prompt_planner_node)

install(TARGETS ${PROJECT_NAME}
  EXPORT prompt_planner_node
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
