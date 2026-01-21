# Plugin Parameters for prompt_bridge

This document describes the common configuration parameters available for a supported plugin in the `prompt_bridge` ROS package. These parameters are typically set in the `config/prompt_bridge.yaml` file.

## Server-Level Parameters

These parameters configure the current available plugin families and their mappings.

| Parameter                  | Description                                       | Example/Default                     |
|----------------------------|--------------------------------------------       |-------------------------------------|
| `frame_id`                 | Frame ID for published history messages.          | `agent`                             |
| `cached_transactions`      | Number of prompt transactions to keep in history. | 10                                  |
| `prompt_family_names`      | List of supported prompt families                 | `[openai, ollama]`                  |
| `prompt_family_plugins`    | Mapping from family name to provider class        | `openai: prompt::OpenAIProvider`    |
|                            |                                                   | `ollama: prompt::OllamaProvider`    |
| `embedding_family_names`   | List of supported embedding families              | `[openai]`                          |
| `embedding_family_plugins` | Mapping from family name to embedding class       | `openai: prompt::OpenAIEmbedding`   |
| `tokenizer_family_names`   | List of supported tokenizer families              | `[openai]`                          |
| `tokenizer_family_plugins` | Mapping from family name to tokenizer class       | `openai: prompt::OpenAITokenize`    |

## Prompting Parameters

These parameters are related to prompting plugins that provide generic communication with different LLMs

| Parameter               | Description                                  | Example/Default                                 |
|-------------------------|----------------------------------------------|-------------------------------------------------|
| `rest.uri`              | The endpoint for completions                 | `https://api.openai.com/v1/completions`         |
| `rest.chat_uri`         | The endpoint for chat completions            | `https://api.openai.com/v1/chat/completions`    |
| `rest.method`           | HTTP method to use                           | `POST`                                          |
| `rest.auth_type`        | Authentication type                          | `Bearer`                                        |
| `rest.ssl_verify`       | Whether to verify SSL certificates           | `true`                                          |
| `option_keys`           | List of option keys available for prompts    | `[stream, model]`                               |
| `options.stream.value`  | Whether to stream responses                  | `false`                                         |
| `options.stream.type`   | Data type for stream option                  | `bool`                                          |
| `options.model.value`   | Default model to use                         | `gpt-5`                                         |
| `options.model.type`    | Data type for model option                   | `string`                                        |

## Embedding Parameters

These parameters are related to embedding plugins that provide communication with different embedding models

| Parameter                      | Description                            | Example/Default                                 |
|--------------------------------|----------------------------------------|-------------------------------------------------|
| `rest.embedding_uri`           | The endpoint for embeddings            | `https://api.openai.com/v1/embeddings`          |
| `rest.method`                  | HTTP method to use                     | `POST`                                          |
| `rest.auth_type`               | Authentication type                    | `Bearer`                                        |
| `rest.ssl_verify`              | Whether to verify SSL certificates     | `true`                                          |
| `option_keys`                  | List of option keys available          | `[model, dimensions, encoding_format]`          |
| `options.model.value`          | Default embedding model                | `text-embedding-3-small`                        |
| `options.model.type`           | Data type for model option             | `string`                                        |
| `options.dimensions.value`     | Default embedding dimensions           | `1536`                                          |
| `options.dimensions.type`      | Data type for dimensions option        | `int`                                           |
| `options.encoding_format.value`| Default encoding format                | `float`                                         |
| `options.encoding_format.type` | Data type for encoding format option   | `string`                                        |

## Tokenizing Parameters

These parameters are related to Tokenizer plugins that provide an interface for tools that facilitate tokenizing

| Parameter               | Description                                   | Example/Default                                 |
|-------------------------|-----------------------------------------------|-------------------------------------------------|
| `option_keys`           | List of option keys available for tokenizing  | `[model]`                                       |
| `options.model.value`   | Default tokenizer model                       | `O200K_BASE`                                    |
| `options.model.type`    | Data type for model option                    | `string`                                        |

---

For more details, see the main README and the example configuration in `config/prompt_bridge.yaml`.