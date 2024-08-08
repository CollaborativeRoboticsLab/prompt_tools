cmake_minimum_required(VERSION 3.10)
project(prompt_schemes)

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(behaviortree_cpp REQUIRED)

# Include directories
include_directories(
  include
  ${tinyxml2_INCLUDE_DIRS}
  )

# ament_target_dependencies(${PROJECT_NAME}
#   rclcpp
#   behaviortree_cpp
# )

install(DIRECTORY include/
  DESTINATION include
)

# Export library
ament_export_include_directories(include)
ament_package()
