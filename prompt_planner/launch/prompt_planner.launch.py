'''
prompt planner launch file
'''
import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    """Generate launch description for prompt planner

    Returns:
        LaunchDescription: The launch description for prompt planner
    """
    # load config file
    planner_config = os.path.join(get_package_share_directory('prompt_planner'), 'config', 'prompt_planner.yaml'
    )

    # create planner composition
    prompt_planner = ComposableNodeContainer(
        name='prompt_planner_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            ComposableNode(
                package='prompt_planner',
                plugin='prompt_planner::PromptPlanner',
                name='prompt_planner',
                parameters=[planner_config]
            )
        ]
    )

    # return
    return LaunchDescription([
        prompt_planner,
    ])
