cmake_minimum_required(VERSION 3.10)
project(prompt_openai)

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

include_directories(include)

# Integrate cpp-tiktoken library
set(CPP_TIKTOKEN_INSTALL OFF CACHE BOOL "Disable install for cpp-tiktoken" FORCE)
set(CPP_TIKTOKEN_TESTING OFF CACHE BOOL "Disable tests for cpp-tiktoken" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Add tiktoken external dependency
add_subdirectory(${CMAKE_SOURCE_DIR}/external/cpp-tiktoken)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/external/cpp-tiktoken)

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
  tiktoken
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
ament_export_libraries(${PROJECT_NAME} tiktoken)

# Install tiktoken model files for runtime. cpp-tiktoken does not install these by itself.
# cpp-Tiktoken expects the model files to be in a 'tokenizers' folder relative to the executable.
# Eventhough we are building the prompt_openai library here, the executables will be in prompt_bridge package.
# So we install the model files to prompt_bridge/lib/prompt_bridge/tokenizers.
install(FILES
  external/cpp-tiktoken/o200k_base.tiktoken
  external/cpp-tiktoken/cl100k_base.tiktoken
  external/cpp-tiktoken/p50k_base.tiktoken
  external/cpp-tiktoken/r50k_base.tiktoken
  external/cpp-tiktoken/qwen.tiktoken
  external/cpp-tiktoken/tokenizer.model
  external/cpp-tiktoken/tokenizer_llama3.1.model
  DESTINATION ../prompt_bridge/lib/prompt_bridge/tokenizers)

ament_package()
