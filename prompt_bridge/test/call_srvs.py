'''
test prompt_bridge by call_srvs
'''

import rclpy
from prompt_msgs.srv import Prompt
from prompt_msgs.msg import ModelOption


def call_prompt_srv(node):
    # create service client
    client = node.create_client(
        Prompt,
        '/prompt_bridge/prompt'
    )

    client.wait_for_service()

    # create a request
    req = Prompt.Request()
    req.prompt.prompt = 'where is the moon?'

    # fill model opts
    # select model
    model_opts = ModelOption()
    model_opts.key = "model"
    model_opts.value = "llama3.2"
    req.prompt.options.append(model_opts)
    # set stream false
    model_opts = ModelOption()
    model_opts.key = "stream"
    model_opts.value = "false"
    model_opts.type = ModelOption.BOOL_TYPE
    req.prompt.options.append(model_opts)

    # call the service
    future = client.call_async(req)

    rclpy.spin_until_future_complete(node, future)

    print(future.result())


if __name__ == '__main__':
    rclpy.init(args=None)

    node = rclpy.create_node('test_prompt_bridge')

    call_prompt_srv(node)
