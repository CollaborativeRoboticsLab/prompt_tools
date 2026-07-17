
# OpenAI Prompt Provider Plugins

This package provides prompt, embedding, and tokenization plugins for OpenAI. The current prompt implementation targets the OpenAI Responses API, embeddings use the Embeddings API, and tokenization is provided locally through [cpp-tiktoken](https://github.com/gh-markt/cpp-tiktoken).

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

Providers are loaded through `prompt_bridge/config/prompt_bridge.yaml`. Example plugin sections:

```yaml
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
    embedding_uri: https://api.openai.com/v1/embeddings
    method: POST
    auth_type: Bearer
    ssl_verify: true
  option_keys: [model, dimensions, encoding_format]
  options:
    model:
      value: text-embedding-3-small
      type: string
    dimensions:
      value: 1536
      type: int
    encoding_format:
      value: float
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

- Default model options are loaded from ROS parameters and filled in when a request omits them.

- Tokenization uses cpp-tiktoken locally, not via REST.

- `OPENAI_API_KEY` must be present in the environment before `prompt_bridge` loads either `OpenAIProvider` or `OpenAIEmbedding`.

- cpp-Tiktoken expects the model files to be in a 'tokenizers' folder relative to the executable. Eventhough prompt_openai plugins CMakeLists.txt lives here, the executables will be in prompt_bridge package. So tiktoken model/data files are installed into `install/prompt_bridge/lib/prompt_bridge` manually via `prompt_openai/CMakeLists.txt`.