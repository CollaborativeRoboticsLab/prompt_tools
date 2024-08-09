cmake_minimum_required(VERSION 3.10.2)
project(prompt_msgs)

set(CMAKE_CXX_STANDARD 17)

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
)

set(srv_files
  "srv/Prompt.srv"
)

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  ${srv_files}
  DEPENDENCIES std_msgs
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
