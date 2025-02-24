# prompt provider plugins

This directory contains prompt provider plugins for the prompt bridge.

## Plugins

- `RestPromptProvider`: A prompt provider that fetches prompts using a REST API.

## Proxy LLM

If not connecting to a Online API, a local LLM running on docker can be used. Separately clone a repository such as 
[CollaborativeRoboticsLab/ollama-docker](https://github.com/CollaborativeRoboticsLab/ollama-docker) for this purpose.

## Usage

Providers are loaded from config. The following example shows how to load the `RestPromptProvider`:

```yaml
prompt_provider_plugin: prompt_provider::RestPromptProvider

prompt_provider::RestPromptProvider:
  url: http://localhost:8000/prompts
  method: POST
```
