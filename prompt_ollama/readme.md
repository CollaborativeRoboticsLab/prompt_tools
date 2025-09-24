# Ollama prompt provider plugins

This directory contains prompt plugins for Ollama API running on host computer or as a docker container.

## Plugins

| Provider | Description |
| --- | --- |
| `SingleOllamaProvider` | A prompt provider plugin that fetches single prompts using a REST API. |
| `ChatOllamaProvider` | A prompt provider plugin that fetches conversation prompts using a REST API. |

## Usage

Providers are loaded from config. The following example shows how to load the `ChatOllamaProvider` or `SingleOllamaProvider`:

```yaml
# comment everything else except the one needed
prompt_provider: prompt::ChatOllamaProvider
prompt_provider: prompt::SingleOllamaProvider

SingleOllamaProvider:
  rest:
    uri: http://10.0.0.246:11434/api/generate # local ollama container
    # uri: http://ollama:11434/api/generate             # docker container with name 'ollama' running on the same computer
    # uri: http://ollama:11434/v1/completions           # docker container with name 'ollama' and openai api compat version
    method: POST
    auth_type: Bearer
    ssl_verify: false
  override_model_options: true
  prompt_option_keys: [stream, model]
  prompt_options:
    stream:
      value: false
      type: bool
    model:
      value: llama3.2
      type: string

ChatOllamaProvider:
  rest:
    uri: http://10.0.0.246:11434/api/chat # local ollama container
    # uri: http://ollama:11434/api/generate             # docker container with name 'ollama' running on the same computer
    # uri: http://ollama:11434/v1/completions           # docker container with name 'ollama' and openai api compat version
    method: POST
    auth_type: Bearer
    ssl_verify: false
  override_model_options: true
  prompt_option_keys: [stream, model]
  prompt_options:
    stream:
      value: false
      type: bool
    model:
      value: llama3.2
      type: string
```

## Parameters

| Parameter | Default Value | Description |
| --------- | ------------- | ----------- |
| rest.uri  | http://localhost:8000/api/v1/prompt  | Api endpoint of the service (including ip and port)     |
| rest.method | POST | REST method to use |
| rest.ssl_verify | True | Whether to verify ssl |
| rest.auth_type  | Bearer | Autherntication Token type | 
| override_model_options | True | whether to override model options in service message |
| prompt_option_keys |  | list of parameters to override |
| prompt_options.x.value |  | value of the parameter | 
| prompt_options.x.type |  | type of the parameter | 

### Current Prompt Options

| Option | Type | Description |
| ------ | ---- | ----------- |
| stream | bool | Whether to stream messages (not supported yet) |
| model  | string | What model to use. Look provider documentation for possible values |