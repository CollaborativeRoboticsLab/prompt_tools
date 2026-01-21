
# Ollama Prompt Provider Plugins

This package provides prompt plugins for the Ollama API, supporting both single-turn and chat-based prompt completion via REST. Ollama must be running locally or in a Docker container.

## Plugins

The following plugin class is available (see `plugins.xml` and `include/prompt_ollama/`):

### `prompt::OllamaProvider`

- Provides prompt interface
- Inherits from PromptBaseClass and overrides JSON conversion functions to be compatible with Ollama json architecture.

## Usage

Providers are loaded from config. Example configuration for prompt providers:

```yaml
# comment everything else except the one needed
prompt_provider: prompt::OllamaProvider

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
- Model options can be overridden via config.