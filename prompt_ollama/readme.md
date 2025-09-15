# prompt provider plugins

This directory contains prompt provider plugins for the prompt bridge.

## Plugins

| Provider | Description |
| --- | --- |
| `SinglePromptProvider` | A prompt provider that fetches single prompts using a REST API. |
| `ChatPromptProvider` | A prompt provider that fetches conversation prompts using a REST API. |


## Usage

Providers are loaded from config. The following example shows how to load the `RestPromptProvider`:

```yaml
prompt_provider: prompt_provider::rest::SinglePromptProvider

rest:
  SinglePromptProvider:
    uri: http://10.0.0.246:11434/api/generate
    # uri: http://ollama:11434/api/generate docker container with name 'ollama'
    # uri: http://ollama:11434/v1/completions (openai api compat version)
    method: POST
    auth_type: Bearer

  ChatPromptProvider:
    uri: http://10.0.0.246:11434/api/chat
    # uri: http://ollama:11434/api/chat docker container with name 'ollama'
    # uri: http://ollama:11434/v1/completions (openai api compat version)
    method: POST
    auth_type: Bearer
```
