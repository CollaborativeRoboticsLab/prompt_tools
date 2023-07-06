# Design

The prompt tools are designed to leverage the resource model design of a ROS system to provide a distributed data interface designed for model prompts and robots. The tools are intended to be used predominantly with large language models (LLMs) and their responses translated into native ROS data types.

A ROS system is a distributed system of nodes that communicate with each other using messages, services, and actions. These data types are defined in ROS message packages. The ROS data types are more structured than the data types used by LLMs. These tools provide a more structured prompt and response system for LLMs which can be interpreted by subsystems in a ROS network, and used to perform robotic tasks.

Typical robotic tasks include navigation, manipulation, and perception. These tasks are performed by subsystems in a ROS network and are managed through structured interfaces. The prompt tools provide translations to these structured interfaces for an LLM to perform robotic tasks.

The advantage of using the prompt tools is that the LLM can be used to perform robotic tasks without having to be trained on the specific robotic tasks, or the specific robot. The LLM can be trained on a large corpus of data, and then used to perform robotic tasks by translating the LLM's prompts and responses into ROS data types that can be distributed to the relevant control system and commission a series of robotic actions or behaviours.

The prompt tools are designed to be used with a ROS system that has a resource model. The resource model is a model of the resources available to the ROS system. The resource model is used to determine which subsystems are available to perform robotic tasks.

## Capability Model

The resource model is a capability model. The capability model is a model of the capabilities of the ROS system. The capabilities are defined by the subsystems in the ROS system. Capabilities can be registered and deregistered by the subsystems for translating to prompts and responses. The capabilities are used to determine which subsystems are available to perform robotic tasks.

## Translation

Translation is completed by a bridge server. The server loads different profiles for different models and translates ROS data types into prompts and vice versa using simple rules.

An example prompt from a user or supervisory control or fleet management system may be:

```text
"Navigate to the kitchen."
```

The bridge server may augment this prompt before passing it to a languange model. The server may encode a resource model into the prompt. For example:

```text
"
you are a robot with the following capabilities:

1. 'move' - command '/move <speed> <direction>' - a type of move command - move at a speed in a direction
2. 'grasp' - command '/grasp <object>' - a type of grasp command - grasp an object
3. 'navigate' - command '/navigate <waypoint>' - a type of navigate command - plan a path task
4. 'search' - command '/search <place>' - a type of search command - search for a place
5. `open` - command '/open <door>' - a type of open command - open a door

to invoke a capability encode a response using the associated command.

using the above information, organise a procedure to achieve the following, estimate the confidence of the procedure as a variable called CONFIDENCE between 0 and 1, and only provide the procedure in bash:

navigate to the kitchen.
"
```

The language model may respond with:

```bash
# Search for the kitchen
search kitchen

# Retrieve the location of the kitchen
kitchen_location=$(retrieve_location)

# Plan a path to the kitchen
navigate $kitchen_location

# Follow the planned path
follow_path

# Optionally, handle any encountered doorways
if doorway_encountered; then
    open door
fi
```

The bridge server may then translate this response into a ROS action sent to the registered type based on commands in the detected in the response. For example:

```text
send action to subsystem for command 'search' with argument 'kitchen'
```

The response of this action will be monitored and reported back to the LLM as a new prompt. For example:

```text
"
the 'search' command has been completed with the following response:

kitchen found at location (x, y, z)

what is the next command?
"
```

and so on until the procedure is completed.
