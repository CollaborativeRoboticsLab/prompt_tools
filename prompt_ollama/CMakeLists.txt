cmake_minimum_required(VERSION 3.10)
project(prompt_ollama)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(pluginlib REQUIRED)
find_package(prompt_msgs REQUIRED)
find_package(prompt_base REQUIRED)
find_package(Poco REQUIRED COMPONENTS
  JSON
  Net
  NetSSL
  Util
  Foundation
)

# Include directories
include_directories(include)

# Library
add_library(${PROJECT_NAME} SHARED
  src/prompt_provider.cpp
)

target_link_libraries(${PROJECT_NAME}
  Poco::JSON
  Poco::Net
  Poco::NetSSL
  Poco::Util
  Poco::Foundation
)

ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  pluginlib
  prompt_msgs
  prompt_base
)

# Plugin description file
pluginlib_export_plugin_description_file(prompt_bridge plugins.xml)

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
