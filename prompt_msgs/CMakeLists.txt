cmake_minimum_required(VERSION 3.10.2)
project(prompt_msgs)

find_package(catkin REQUIRED COMPONENTS
  std_msgs
  message_generation
)

add_message_files(
  FILES
  ModelOptions.msg
  Prompt.msg
  PromptResponse.msg
  PromptStamped.msg
  PromptResponseStamped.msg
  PromptTransaction.msg
  PromptHistory.msg
)

add_service_files(
  FILES
  Prompt.srv
)

generate_messages(
  DEPENDENCIES
  std_msgs
)

catkin_package(CATKIN_DEPENDS message_runtime std_msgs)
