'''
test prompt_bridge by call actions
'''

import rclpy
from rclpy.action import ActionClient
from prompt_msgs.action import Plan
from prompt_msgs.msg import ModelOption


def call_plan_action(node):
    # create action client
    action_client = ActionClient(node, Plan, '/prompt_bridge/plan')

    action_client.wait_for_server()

    goal_msg = Plan.Goal()
    goal_msg.goal.prompt.prompt = 'where is the moon'

    # fill model opts
    # select model
    model_opts = ModelOption()
    model_opts.key = "model"
    model_opts.value = "llama3.1:8b"
    goal_msg.goal.prompt.options.append(model_opts)
    # set stream false
    model_opts = ModelOption()
    model_opts.key = "stream"
    model_opts.value = "false"
    model_opts.type = ModelOption.BOOL_TYPE
    goal_msg.goal.prompt.options.append(model_opts)

    goal_future = action_client.send_goal_async(
        goal_msg,
        feedback_callback=lambda f: print(f.feedback)
    )

    rclpy.spin_until_future_complete(node, goal_future)

    if goal_future.result() is not None:
        print(goal_future.result())
    else:
        raise RuntimeError(
            f'Goal with action client call failed : {goal_future.exception()}'
        )


if __name__ == '__main__':
    rclpy.init(args=None)

    node = rclpy.create_node('test_prompt_bridge')

    call_plan_action(node)
