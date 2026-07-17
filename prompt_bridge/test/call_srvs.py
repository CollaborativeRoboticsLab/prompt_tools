'''
test prompt_bridge by calling the current prompt service
'''

import rclpy
from prompt_msgs.srv import Prompt
from prompt_msgs.msg import ModelOption


def call_prompt_srv(node):
    # create service client
    client = node.create_client(
        Prompt,
        'prompt/prompt'
    )

    client.wait_for_service()

    # create a request
    req = Prompt.Request()
    req.uuid = ''
    req.prompt.prompt = 'where is the moon?'
    req.prompt.use_cache = False
    req.prompt.flush_cache = False
    req.prompt.use_chat_mode = False
    req.prompt.model_family = 'ollama'

    # fill model opts
    # select model
    model_opts = ModelOption()
    model_opts.key = 'model'
    model_opts.value = 'llama3.2'
    model_opts.type = ModelOption.STRING_TYPE
    req.prompt.options.append(model_opts)

    # set stream false
    model_opts = ModelOption()
    model_opts.key = 'stream'
    model_opts.value = 'false'
    model_opts.type = ModelOption.BOOL_TYPE
    req.prompt.options.append(model_opts)

    # call the service
    future = client.call_async(req)

    rclpy.spin_until_future_complete(node, future)

    result = future.result()
    if result is None:
        raise RuntimeError(f'Prompt service call failed: {future.exception()}')

    print(result)


if __name__ == '__main__':
    rclpy.init(args=None)

    node = rclpy.create_node('test_prompt_bridge')

    call_prompt_srv(node)
