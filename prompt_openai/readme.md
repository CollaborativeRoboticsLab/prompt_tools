# OpenAI prompt provider plugins

This directory contains prompt plugins for OpenAI API.

## Plugins

| Provider | Description |
| --- | --- |
| `SingleOpenAIProvider` | A prompt provider plugin that fetches single prompts using a REST API. |
| `ChatOpenAIProvider` | A prompt provider plugin that fetches conversation prompts using a REST API. |

## Usage

Providers are loaded from config. The following example shows how to load the `ChatOpenAIProvider` or `SingleOpenAIProvider`:

```yaml
# comment everything else except the one needed
prompt_provider: prompt::ChatOpenAIProvider
prompt_provider: prompt::SingleOpenAIProvider

ChatOpenAIProvider:
  rest:
    uri: https://api.openai.com/v1/chat/completions # openai endpoint for gpt models
    method: POST
    auth_type: Bearer
    ssl_verify: true
  override_model_options: true
  prompt_option_keys: [stream, model]
  prompt_options:
    stream:
      value: false
      type: bool
    model:
    #   value: gpt-4o
    #   value: gpt-4.1
      value: gpt-5
      type: string

SingleOpenAIProvider:
  rest:
    uri: https://api.openai.com/v1/completions # openai endpoint for gpt models
    method: POST
    auth_type: Bearer
    ssl_verify: true
  override_model_options: true
  prompt_option_keys: [stream, model]
  prompt_options:
    stream:
      value: false
      type: bool
    model:
    #   value: gpt-4o
    #   value: gpt-4.1
      value: gpt-5
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
