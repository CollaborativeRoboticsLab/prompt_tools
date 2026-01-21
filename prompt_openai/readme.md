
# OpenAI Prompt Provider Plugins

This package provides prompt, embedding, and tokenization plugins for the OpenAI API, supporting both chat and completion endpoints, as well as embeddings via REST and tokenization via [cpp-tiktoken](https://github.com/gh-markt/cpp-tiktoken).

## Plugins

The following plugin classes are available (see `plugins.xml` and `include/prompt_openai/`):

### `prompt::OpenAIProvider`

- Provides Prompt interface
- Inherits from PromptBaseClass and overrides JSON conversion functions to be compatible with OpenAI json architecture.

### `prompt::OpenAIEmbedding`

- Provides Embedding interface
- Inherits from EmbedBaseClass and overrides JSON conversion functions to be compatible with OpenAI json architecture.

### `prompt::OpenAITokenize`

- Provides Tokenizing interface
- Inherits from TokenizeBaseClass and overrides process function to interact with cpp-tiktoken interface.


## Usage

Providers are loaded from config. Example configuration for prompt providers:

```yaml
# comment everything else except the one needed
prompt_provider: prompt::OpenAIProvider
prompt_provider: prompt::OpenAIEmbedding
prompt_provider: prompt::OpenAITokenize

OpenAIProvider:
  rest:
    # OpenAI Responses API endpoint (used for both single prompts and conversations)
    uri: https://api.openai.com/v1/responses
    chat_uri: https://api.openai.com/v1/responses
    method: POST
    auth_type: Bearer
    ssl_verify: true
  option_keys: [stream, model] # options will be used if model options are not set in the prompt requests
  options:
    stream:
      value: false
      type: bool
    model:
      value: gpt-5    # e.g., gpt-4o, gpt-4.1, gpt-4o-mini
      type: string

OpenAIEmbedding:
  rest:
    uri: https://api.openai.com/v1/embeddings
    method: POST
    auth_type: Bearer
    ssl_verify: true
  override_model_options: true
  option_keys: [model]
  options:
    model:
      value: text-embedding-3-small
      type: string

OpenAITokenize:
  # No REST config needed; uses local cpp-tiktoken for tokenization
  option_keys: [model] # options will be used if model options are not set in the prompt requests
  options:
    model:
      value: O200K_BASE
      type: string

```

## Build & Dependencies

- Depends on `rclcpp`, `pluginlib`, `prompt_msgs`, `prompt_base`, and `Poco` libraries.
- Integrates the [cpp-tiktoken](external/cpp-tiktoken/) library for local tokenization.

## Notes
- See `plugins.xml` and the `include/prompt_openai/` headers for class details.

- Model options can be overridden via config.

- Tokenization uses cpp-tiktoken locally, not via REST.

- cpp-Tiktoken expects the model files to be in a 'tokenizers' folder relative to the executable. Eventhough prompt_openai plugins CMakeLists.txt lives here, the executables will be in prompt_bridge package. So tiktoken model/data files are installed into `install/prompt_bridge/lib/prompt_bridge` manually via `prompt_openai/CMakeLists.txt`.