cmake_minimum_required(VERSION 3.10.2)
project(prompt_msgs)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(std_msgs REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(rosidl_default_generators REQUIRED)

set(msg_files
  "msg/ModelOption.msg"
  "msg/Prompt.msg"
  "msg/PromptResponse.msg"
  "msg/PromptStamped.msg"
  "msg/PromptResponseStamped.msg"
  "msg/PromptTransaction.msg"
  "msg/PromptHistory.msg"
  "msg/Embed.msg"
  "msg/EmbedResponse.msg"
  "msg/Embedding.msg"
  "msg/Token.msg"
  "msg/TokenResponse.msg"
)

set(srv_files
  "srv/Prompt.srv"
  "srv/Embedding.srv"
  "srv/Tokenize.srv"
)

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  ${srv_files}
  DEPENDENCIES std_msgs
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
