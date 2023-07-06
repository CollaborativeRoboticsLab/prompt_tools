# prompt_msgs

Message types for prompted systems such as large language models (LLMs) and their responses in a distributed data driven robotic system application (ROS).

The work has been created as part of a proposed ROS REP.

## Message Types

### Prompt

The `Prompt` message type is used to send prompts to prompted systems such as large language models (LLMs).

```yaml
string prompt
string[] options
```

### PromptResponse

The `PromptResponse` message type is used to send responses from prompted systems such as large language models (LLMs).

```yaml
string response
```
