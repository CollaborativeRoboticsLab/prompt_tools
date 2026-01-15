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

find_package(PkgConfig)
pkg_check_modules(UUID REQUIRED uuid)

include_directories(
  include
  ${UUID_INCLUDE_DIRS}
)


############################################################################
# prompt bridge server executable
############################################################################

add_executable(${PROJECT_NAME}_node
  src/prompt_bridge_node.cpp
)

ament_target_dependencies(${PROJECT_NAME}_node
  rclcpp
  rclcpp_action
  pluginlib
  prompt_msgs
  prompt_base
  UUID
)

install(TARGETS ${PROJECT_NAME}_node
  DESTINATION lib/${PROJECT_NAME}
)

############################################################################
# prompt bridge server component
############################################################################

add_library(${PROJECT_NAME}_comp SHARED
  src/prompt_bridge.cpp
)

ament_target_dependencies(${PROJECT_NAME}_comp
  rclcpp
  rclcpp_components
  rclcpp_action
  pluginlib
  prompt_msgs
  prompt_base
  UUID
)

# register component
rclcpp_components_register_node(${PROJECT_NAME}_comp
  PLUGIN "prompt::PromptBridge"
  EXECUTABLE prompt_bridge_component
)

ament_export_targets(prompt_bridge_component)

install(TARGETS ${PROJECT_NAME}_comp
  EXPORT prompt_bridge_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

############################################################################
# test node for prompt_bridge
############################################################################


# Use .cpp for test node source to ensure CMake detects C++ language
add_executable(test_prompt_node
  test/test_prompt.cpp
)

ament_target_dependencies(test_prompt_node
  rclcpp
  prompt_msgs
)

install(TARGETS test_prompt_node
  DESTINATION lib/${PROJECT_NAME}
)

############################################################################
# Miscellaneous installations
############################################################################

install(DIRECTORY include/
  DESTINATION include
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME}_comp)
ament_package()
