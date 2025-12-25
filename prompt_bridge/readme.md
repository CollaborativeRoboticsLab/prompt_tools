
# prompt_bridge

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

ROS 2 package for bridging between prompted systems (such as large language models) and a ROS robotic system application. Provides a flexible, plugin-based interface for integrating LLMs and other prompted systems into ROS workflows.


## Motivation

`prompt_bridge` is designed to provide a generic, extensible interface for integrating prompted systems (e.g., LLMs) into ROS 2 robotic applications. It follows ROS best practices by using a plugin architecture, allowing different LLM providers to be loaded at runtime. This enables rapid experimentation and integration of new models and providers without changing core code.


## Features

- **Plugin-based architecture:** Easily add new LLM providers or prompt schemes via plugins.
- **Unified ROS interfaces:** Provides ROS services for sending prompts and receiving responses.
- **Prompt history tracking:** Publishes prompt/response history for monitoring and debugging.
- **Chat and cache modes:** Supports conversational (chat) and stateless prompt handling, with optional caching and flushing.
- **Dynamic configuration:** Model families and plugins are loaded at runtime from parameters or YAML config.

## Node: `prompt_bridge_node`

The main node, `prompt_bridge_node`, exposes a unified interface for prompted systems in ROS 2. It loads provider plugins at startup and manages prompt transactions, history, and conversation state.

### ROS Interfaces

- **Service:** `prompt/prompt` ([prompt_msgs/srv/Prompt](prompt_msgs/srv/Prompt.srv))
    - Main entry point for sending prompts and receiving responses.
- **Publisher:** `prompt/history` ([prompt_msgs/msg/PromptHistory](prompt_msgs/msg/PromptHistory.msg))
    - Publishes a rolling history of prompt transactions.

### Parameters

| Name                        | Type     | Default      | Description                                                      |
|-----------------------------|----------|--------------|------------------------------------------------------------------|
| `frame_id`                  | string   | `agent`      | Frame ID for published history messages.                         |
| `cached_transactions`       | int      | `10`         | Number of prompt transactions to keep in history.                |
| `model_family_names`        | string[] | `[]`         | List of model family keys to load as plugins.                    |
| `model_family_plugins`      | map      | `{}`         | Mapping from family key to plugin class name.                    |

See `config/prompt_bridge.yaml` for example configuration.

#### Example (YAML):

```yaml
ros__parameters:
  frame_id: prompt_bridge
  cached_transactions: 10
  model_family_names: [openai, ollama]
  model_family_plugins:
    openai: prompt::OpenAIProvider
    ollama: prompt::OllamaProvider
```

### Plugin Parameters

Each plugin (e.g., OpenAIProvider, OllamaProvider) can have its own set of configuration parameters, such as API endpoints, authentication, and default prompt options. For details, see [docs/plugin_parameters](../docs/plugin_parameters.md).


## Usage

1. Configure your desired plugins and parameters in a YAML file (see above).
2. Launch the main node with your config:
    ```bash
    ros2 run prompt_bridge prompt_bridge_node --ros-args --params-file config/prompt_bridge.yaml
    ```
3. Use the provided ROS service to send prompts and receive responses.

## Extending

To add a new Online LLM provider, implement a plugin inheriting from `prompt::RESTBaseClass` and register it. Add its configuration to your YAML file and list it in `model_family_names` and `model_family_plugins`.

## See Also

- [docs/plugin_parameters.md](docs/plugin_parameters.md) — Plugin-specific configuration
- `config/prompt_bridge.yaml` — Example configuration

---

MIT License. See LICENSE file for details.
