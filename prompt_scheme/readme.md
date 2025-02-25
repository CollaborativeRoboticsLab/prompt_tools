# prompt_scheme

The `prompt_scheme` package provides transcoding schemes for natural language prompts to and from ROS `capabilities`. The purpose of the transcoding schemes is to provide a way to translate natural language prompts into robotic actions. The transcoding schemes are designed to be used with a ROS system that has a resource model. For example, the `capabilities` package provides a capability model for a ROS system.

## plugins

The `prompt_scheme` package provides a set of plugins that implement the schemes.

- `BTPlannerScheme` - A scheme that uses the Behavior Tree (BT) framework to transcode natural language prompts into a behavior tree. Tailored to work with ROS2 action service
- `BufferScheme` - A scheme that provides prompt buffering for robotics applications where a response is not expected always. Intends to optimize token usage.

## usage

load schemes into the `prompt_bridge` using config.

```yaml
prompt_scheme: prompt_scheme::BufferScheme

  BufferScheme:
    override: true
    prompt_option_keys: [stream, model]
    prompt_options:
      stream:
        value: false
        type: bool
      model:
        value: llama3.2

  BTPlannerScheme:
    prompt_option_keys: [stream, model]
    prompt_options:
      stream:
        value: false
        type: bool
      model:
        value: llama3.1:8b
```
