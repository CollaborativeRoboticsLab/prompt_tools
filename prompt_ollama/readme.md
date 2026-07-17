
# Ollama Prompt Provider Plugins

This package provides prompt plugins for the Ollama API, supporting both single-turn and chat-based prompting via REST. Ollama must be reachable from the machine running `prompt_bridge`.

## Plugins

The following plugin class is available (see `plugins.xml` and `include/prompt_ollama/`):

### `prompt::OllamaProvider`

- Provides prompt interface
- Inherits from PromptBaseClass and overrides JSON conversion functions to be compatible with Ollama json architecture.

## Usage

Providers are loaded through `prompt_bridge/config/prompt_bridge.yaml`. Example plugin section:

```yaml
OllamaProvider:
  rest:
    uri: http://10.0.0.246:11434/api/generate           # local ollama container. change ip as required
    chat_uri: http://10.0.0.246:11434/api/chat          # local ollama container. change ip as required
    method: POST
    auth_type: Bearer
    ssl_verify: false
  option_keys: [stream, model] # options will be used if model options are not set in the prompt requests
  options:
    stream:
      value: false
      type: bool
    model:
      value: llama3.2
      type: string
```

## Build & Dependencies

- Depends on `rclcpp`, `pluginlib`, `prompt_msgs`, `prompt_base`, and `Poco` libraries.

## Notes
- Only prompt (completion/chat) plugins are provided for Ollama.
- See `plugins.xml` and the `include/prompt_ollama/` header for class details.
- Default model options are loaded from ROS parameters and filled in when a request omits them.
- No API key is required by the bundled Ollama provider.