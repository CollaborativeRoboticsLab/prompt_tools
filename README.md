# Prompt Tools

[![ROS2 Jazzy](https://img.shields.io/badge/ROS2-Jazzy-blue)](https://index.ros.org/doc/ros2/Releases/)
[![ROS2 Humble](https://img.shields.io/badge/ROS2-Humble-blue)](https://index.ros.org/doc/ros2/Releases/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](./LICENSE)
[![Open in Visual Studio Code](https://img.shields.io/badge/vscode-dev-blue)](https://open.vscode.dev/CollaborativeRoboticsLab/prompt_tools)
<!-- [![DOI](https://zenodo.org/badge/DOI/10.1/zenodo.1.svg)](https://doi.org/10.1/zenodo.1) -->

ROS 2 meta-package with tools for working with prompted systems such as large language models and their responses in distributed robotic applications. It provides generic ROS message types for prompt, embedding, and tokenization workflows together with a flexible plugin-based bridge. The current providers are:

| Provider | Package |
| --- | --- |
| OpenAI | [prompt_openai](./prompt_openai/readme.md)  |
| Ollama | [prompt_ollama](./prompt_ollama/readme.md)  |

## Motivation

`prompt_bridge` is designed to provide a generic, extensible interface for integrating prompted systems (e.g., LLMs) into ROS 2 robotic applications. It follows ROS best practices by using a plugin architecture, allowing different LLM providers to be loaded at runtime. This enables rapid experimentation and integration of new models and providers without changing core code.

## Features

- **Plugin-based architecture:** Easily add new LLM providers or prompt schemes via plugins.
- **Unified ROS interfaces:** Provides ROS services for prompt, embedding, and tokenization requests.
- **Prompt history tracking:** Publishes prompt/response history for monitoring and debugging.
- **Chat and cache modes:** Supports conversational (chat) and stateless prompt handling, with optional caching and flushing.
- **Dynamic configuration:** Model families and plugins are loaded at runtime from parameters or YAML config.

<br>

## Prompt Bridge

The main node that connects ROS 2 applications to prompt providers. It loads providers through `pluginlib` and exposes prompt, embedding, and tokenization services concurrently.

- **Prompt Interfaces:** 
    - `prompt/prompt` ([prompt_msgs/srv/Prompt](prompt_msgs/srv/Prompt.srv))
    - Main entry point for sending prompts and receiving responses.

- **Embedding Interfaces:** 
    - `prompt/embedding` ([prompt_msgs/srv/Embedding](prompt_msgs/srv/Embedding.srv))
    - Main entry point for requesting embedding vectors for text.

- **Tokenization interfaces:** 
    - `prompt/tokenizer` ([prompt_msgs/srv/Tokenize](prompt_msgs/srv/Tokenize.srv))
    - Main entry point for encoding text to tokens and decoding tokens back into text.

- **History Publisher:** 
    - `prompt/history` ([prompt_msgs/msg/PromptHistory](prompt_msgs/msg/PromptHistory.msg))
    - Publishes a rolling history of prompt transactions.

Following is the current system Architecture

![system structure](./docs/images/system_architecture.png)

## Read more about,

- [Prompt Interface](./docs/prompt_interface.md) to understand about Prompting Sub System
- [Embed Interface](./docs/embed_interface.md) to understand about Embedding Sub System
- [Tokenize Interface](./docs/tokenize_interface.md) to understand about PromptTokenizer Sub System
- [Class Inheritance](./docs/class_structure.md)  to understand how to inherit when creating new plugins
- [Plugin parameters](./docs/plugin_parameters.md)  to understand how to configure new and existing plugins to change behaviour

<br>

## Install

### Clone packages

Clone the prompt tools package

```bash
cd src
git clone https://github.com/CollaborativeRoboticsLab/prompt_tools.git
```

### Initialize submodules

```bash
cd prompt_tools
git submodule update --init --recursive
```

### Dependency Installation

```bash
sudo apt update && sudo apt install -y libuuid-dev
```

Move to workspace root and run the following command to install dependencies

```bash
cd ../..
rosdep install --from-paths src --ignore-src -r -y
```

<br>

## API Keys

### Using Proxy LLM

If not connecting to a Online API, a local LLM running on docker can be used. Separately clone a repository such as [CollaborativeRoboticsLab/ollama-docker](https://github.com/CollaborativeRoboticsLab/ollama-docker) for this purpose and start it.

### Using OpenAI API

Run the following command with the actual `OPENAI_API_KEY` in place of `<open-ai-api-key>` if using prompt-openai plugins

```bash
export OPENAI_API_KEY="<open-ai-api-key>"
```

Then build the workspace and launch `prompt_bridge` with the packaged YAML configuration. The default OpenAI configuration uses the Responses API for prompts and the Embeddings API for embeddings.

```bash
colcon build
```

### Using the devcontainer

Rename the `.devcontainer/devcontainer-empty.env` as `.devcontainer/devcontainer.env` and update it with your API Keys. Then rebuild the container


<br>

## Usage

### Starting the Prompt Bridge

```bash
source install/setup.bash
ros2 launch prompt_bridge prompt_bridge.launch.py
```

### Testing

To build and run the C++ test node that exercises the current prompt, chat, cache, and embedding flows:

```bash
source install/setup.bash
ros2 run prompt_bridge test_prompt_node
```

This will run the test node and print results for stateless prompting, chat mode, cached prompting, and embedding requests.

### Python Examples

Two lightweight Python examples are available under `prompt_bridge/test/`:

- `call_srvs.py` sends a single request to `prompt/prompt`.
- `call_services.py` exercises `prompt/prompt`, `prompt/embedding`, and `prompt/tokenizer`.

Run them from a sourced workspace after `prompt_bridge` is already running:

```bash
source install/setup.bash
python3 src/prompt_tools/prompt_bridge/test/call_srvs.py
python3 src/prompt_tools/prompt_bridge/test/call_services.py
```

## Current Defaults

- Prompt service: `prompt/prompt`
- Embedding service: `prompt/embedding`
- Tokenizer service: `prompt/tokenizer`
- History topic: `prompt/history`
- Default config: `prompt_bridge/config/prompt_bridge.yaml`

OpenAI prompt requests are sent to `https://api.openai.com/v1/responses`, OpenAI embedding requests are sent to `https://api.openai.com/v1/embeddings`, and OpenAI tokenization is handled locally through `cpp-tiktoken`.

## Citation

If you use this work in an academic context, please cite the following publication(s):

```bibtex
@misc{ratnayake2026gpsfsm,
  title={A Generative Partially Specified Finite State Machine Approach to Complex Behaviour Planning}, 
  author={Kalana Ratnayake and Michael Pritchard and David Hinwood and Maleen Jayasuriya and Damith Herath},
  year={2026},
  eprint={2607.15674},
  archivePrefix={arXiv},
  primaryClass={cs.RO},
  url={https://arxiv.org/abs/2607.15674}, 
}
```
