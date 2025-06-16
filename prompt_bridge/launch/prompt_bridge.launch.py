'''
capabilities2_server launch file
'''

import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    """Generate launch description for prompt bridge server

    Returns:
        LaunchDescription: The launch description for prompt bridge server
    """
    # load config file
    server_config = os.path.join(get_package_share_directory('prompt_bridge'), 'config', 'prompt_bridge.yaml')

    # create bridge composition
    prompt_bridge = Node(
        package='prompt_bridge',
        executable='prompt_bridge_node',
        name='prompt_bridge',
        parameters=[server_config],
        output='screen',
        arguments=['--ros-args', '--log-level', 'info']
    )

    # return
    return LaunchDescription([
        prompt_bridge
    ])
