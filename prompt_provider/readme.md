# prompt provider plugins

This directory contains prompt provider plugins for the prompt bridge.

## Plugins

- `SinglePromptProvider`: A prompt provider that fetches single prompts using a REST API.
- `ChatPromptProvider`: A prompt provider that fetches conversation prompts using a REST API.

## Usage

Providers are loaded from config. The following example shows how to load the `RestPromptProvider`:

```yaml
prompt_provider_plugin: prompt_provider::RestPromptProvider

prompt_provider::RestPromptProvider:
  url: http://localhost:8000/prompts
  method: POST
```
