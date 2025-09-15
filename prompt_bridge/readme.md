# prompt_bridge

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

ROS package for bridging between prompted systems such as large language models and a ROS robotic system application.

## Motivation

This package is part of a proposed ROS REP. Following the principle design purpose of ROS itself - to stop reinventing the wheel - this package aims to provide a generic interface for prompted systems such as large language models (LLMs) to be used in a ROS robotic system application. The package provides a ROS resource interface and plugin architecture much like other packages such as the navigation stack for LLM prompts and different LLM providers. This package contains a bridge node to connect prompted systems to ROS.

## API

### prompt_bridge_node

The `prompt_bridge_node` is a ROS node that connects prompted systems to ROS.

It provides ROS topic, service, and action interfaces for prompted systems to send prompts to and receive responses from.

The bridge uses plugins for different models and providers. The plugins are loaded at runtime.

### Parameters

* `~prompt_service` (string, default: `prompt`)

    The name of the service to be used for sending prompts to the prompted system.

### Subscribed Topics

* `~prompt` ([prompt_msgs/Prompt](prompt_msgs/msg/Prompt.msg))

    The prompt to be sent to the prompted system.

### Published Topics

* `~response` ([prompt_msgs/Response](prompt_msgs/msg/Response.msg))

    The response from the prompted system.

### Services

* `~prompt` ([prompt_msgs/Prompt](prompt_msgs/srv/Prompt.srv))

    The prompt to be sent to the prompted system.

### Actions

* `~prompt` ([prompt_msgs/Prompt](prompt_msgs/action/Prompt.action))

    The prompt to be sent to the prompted system.
