'''
exercise the current prompt_bridge services

This example calls the prompt, embedding, and tokenizer services that are
currently implemented.
'''

import rclpy
from prompt_msgs.srv import Embedding, Prompt, Tokenize
from prompt_msgs.msg import ModelOption


def call_prompt_service(node):
    client = node.create_client(Prompt, 'prompt/prompt')
    client.wait_for_service()

    req = Prompt.Request()
    req.uuid = ''
    req.prompt.prompt = 'Summarize why the moon appears bright.'
    req.prompt.use_cache = False
    req.prompt.flush_cache = False
    req.prompt.use_chat_mode = False
    req.prompt.model_family = 'ollama'

    model_opts = ModelOption()
    model_opts.key = 'model'
    model_opts.value = 'llama3.2'
    model_opts.type = ModelOption.STRING_TYPE
    req.prompt.options.append(model_opts)

    model_opts = ModelOption()
    model_opts.key = 'stream'
    model_opts.value = 'false'
    model_opts.type = ModelOption.BOOL_TYPE
    req.prompt.options.append(model_opts)

    future = client.call_async(req)
    rclpy.spin_until_future_complete(node, future)

    result = future.result()
    if result is None:
        raise RuntimeError(f'Prompt service call failed: {future.exception()}')

    print('Prompt response:', result)


def call_embedding_service(node):
    client = node.create_client(Embedding, 'prompt/embedding')
    client.wait_for_service()

    req = Embedding.Request()
    req.input.text = 'The moon reflects sunlight.'
    req.input.model_family = 'openai'

    model_opt = ModelOption()
    model_opt.key = 'model'
    model_opt.value = 'text-embedding-3-small'
    model_opt.type = ModelOption.STRING_TYPE
    req.input.options.append(model_opt)

    format_opt = ModelOption()
    format_opt.key = 'encoding_format'
    format_opt.value = 'float'
    format_opt.type = ModelOption.STRING_TYPE
    req.input.options.append(format_opt)

    future = client.call_async(req)
    rclpy.spin_until_future_complete(node, future)

    result = future.result()
    if result is None:
        raise RuntimeError(f'Embedding service call failed: {future.exception()}')

    print('Embedding response:', result)


def call_tokenizer_service(node):
    client = node.create_client(Tokenize, 'prompt/tokenizer')
    client.wait_for_service()

    req = Tokenize.Request()
    req.input.text = 'Hello moon'
    req.input.tokens = []
    req.input.encode = True
    req.input.model_family = 'openai'

    model_opt = ModelOption()
    model_opt.key = 'model'
    model_opt.value = 'O200K_BASE'
    model_opt.type = ModelOption.STRING_TYPE
    req.input.options.append(model_opt)

    future = client.call_async(req)
    rclpy.spin_until_future_complete(node, future)

    result = future.result()
    if result is None:
        raise RuntimeError(f'Tokenizer service call failed: {future.exception()}')

    print('Tokenizer response:', result)


if __name__ == '__main__':
    rclpy.init(args=None)

    node = rclpy.create_node('test_prompt_bridge')

    try:
        call_prompt_service(node)
        call_embedding_service(node)
        call_tokenizer_service(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()
