cmake_minimum_required(VERSION 3.10)
project(llm_prompt_provider_plugins)

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(pluginlib REQUIRED)
find_package(prompt_msgs REQUIRED)

# Include directories
include_directories(include)

# Library
add_library(${PROJECT_NAME} SHARED
  src/prompt_provider.cpp
)

ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  pluginlib
  prompt_msgs
)

# Plugin description file
pluginlib_export_plugin_description_file(prompt_provider_plugins prompt_provider_plugins.xml)

install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include
)

# Export library
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_package()
