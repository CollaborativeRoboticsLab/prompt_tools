# Plugin Parameters for prompt_bridge

This document describes the common configuration parameters available for a supported plugin in the `prompt_bridge` ROS package. These parameters are typically set in the `config/prompt_bridge.yaml` file.

## Generic Parameters


| Parameter  | Description      | Example/Default     |
|------------|------------------|---------------------|
| `rest.uri`                     | The endpoint for completions                                       | `https://api.openai.com/v1/completions`         |
| `rest.chat_uri`                | The endpoint for chat completions                                  | `https://api.openai.com/v1/chat/completions`    |
| `rest.method`                  | HTTP method to use                                                 | `POST`                                          |
| `rest.auth_type`               | Authentication type                                                | `Bearer`                                        |
| `rest.ssl_verify`              | Whether to verify SSL certificates                                 | `true`                                          |
| `prompt_option_keys`           | List of option keys available for prompts                          | `[stream, model]`                               |
| `prompt_options.stream.value`  | Whether to stream responses                                        | `false`                                         |
| `prompt_options.stream.type`   | Data type for stream option                                        | `bool`                                          |
| `prompt_options.model.value`   | Default model to use                                               | `gpt-5`                                         |
| `prompt_options.model.type`    | Data type for model option                                         | `string`                                        |

---

For more details, see the main README and the example configuration in `config/prompt_bridge.yaml`.