# prompt_schemes

The `prompt_schemes` package provides transcoding schemes for natural language prompts to and from ROS `capabilities`. The purpose of the transcoding schemes is to provide a way to translate natural language prompts into robotic actions. The transcoding schemes are designed to be used with a ROS system that has a resource model. For example, the `capabilities` package provides a capability model for a ROS system.

## plugins

The `prompt_schemes` package provides a set of plugins that implement the schemes.

- `BTScheme` - A scheme that uses the Behavior Tree (BT) framework to transcode natural language prompts into a behavior tree.

## usage

load schemes into the `prompt_bridge` using config.

```yaml
prompt_bridge:
  scheme: prompt_schemes::BTScheme
```
