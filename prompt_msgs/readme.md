# prompt_msgs

Message service and action types for prompted systems such as large language models (LLMs) and their responses translated into distributed data driven robotic system applications (ROS).

The work has been created as part of a proposed ROS REP.

## Message Types

- `ModelOptions` - A message type for a list of options for a model.
- `Prompt.msg` - A message type for a prompt to be sent to an LLM.
- `PromptResponse.msg` - A message type for a response from an LLM.
- `PromptStamped.msg` - Stamped version of `Prompt.msg`.
- `PromptResponseStamped.msg` - Stamped version of `PromptResponse.msg`.
- `PromptTransaction.msg` - A message type for a prompt and response transaction.
- `PromptHistory.msg` - A message type for a history of prompt and response transactions.
- `Capability.msg` - A message type for a capability of a robot.
- `CapabilityResponse.msg` - A message type for a response from a robot related to a capability.
- `Command.msg` - A message type for a command to a robot.

## Service Types

- `Prompt.srv` - A service type for a prompt to be sent and recieved to a model.

## Action Types

- `Command.action` - An action type for a command to be sent to a robot.
