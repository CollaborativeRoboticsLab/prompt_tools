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
    bridge_config = os.path.join(
        get_package_share_directory('prompt_bridge'),
        'config',
        'prompt_bridge.yaml'
    )

    cap_config = os.path.join(
        get_package_share_directory('capabilities2_server'),
        'config',
        'capabilities.yaml'
    )

    # components
    bridge_component = ComposableNode(
        package='prompt_bridge',
        plugin='prompt_bridge::PromptBridge',
        name='prompt_bridge',
        parameters=[bridge_config]
    )

    cap_component = ComposableNode(
        package='capabilities2_server',
        plugin='capabilities2_server::CapabilitiesServer',
        name='capabilities',
        parameters=[cap_config]
    )

    # create bridge composition
    prompt_bridge = ComposableNodeContainer(
        name='prompt_bridge_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            bridge_component,
            # cap_component
        ]
    )

    # create launch proxy node
    launch_proxy = Node(
        package='capabilities2_launch_proxy',
        executable='capabilities_launch_proxy',
        name='capabilities_launch_proxy'
    )

    # return
    return LaunchDescription([
        prompt_bridge,
        # launch_proxy
    ])
