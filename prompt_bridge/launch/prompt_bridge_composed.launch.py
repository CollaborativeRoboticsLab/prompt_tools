'''
prompt bridge launch file
'''
import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    """Generate launch description for prompt bridge

    Returns:
        LaunchDescription: The launch description for prompt bridge
    """
    # load config file
    bridge_config = os.path.join(get_package_share_directory('prompt_bridge'), 'config', 'prompt_bridge.yaml'
    )

    # create bridge composition
    prompt_bridge = ComposableNodeContainer(
        name='prompt_bridge_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container_mt',
        output='screen',
        arguments=['--ros-args', '--log-level', 'info'],
        composable_node_descriptions=[
            ComposableNode(
                package='prompt_bridge',
                plugin='prompt::PromptBridge',
                name='prompt_bridge',
                parameters=[bridge_config]
            )
        ]
    )

    # return
    return LaunchDescription([
        prompt_bridge,
    ])
