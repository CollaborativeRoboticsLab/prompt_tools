cmake_minimum_required(VERSION 3.10.2)
project(prompt_bridge)

find_package(catkin REQUIRED COMPONENTS
prompt_msgs
llm_prompt_provider_plugins
)

catkin_package(CATKIN_DEPENDS)

# include directories
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

# bridge server node
add_executable(prompt_bridge_server src/prompt_bridge_node.cpp)

target_link_libraries(prompt_bridge_server
${catkin_LIBRARIES}
)
