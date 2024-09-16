#pragma once

#include <capabilities2_runner/action_runner.hpp>
#include <pluginlib/class_list_macros.hpp>
#include <prompt_msgs/action/prompt.hpp>

namespace prompt_bridge
{
/**
 * @brief prompt capability runner
 *
 * this class is a wrapper around the capabilities2 action runner
 * it is used to run the prompt capability
 *
 */
class PromptRunner : public capabilities2_runner::ActionRunner<prompt_msgs::action::Prompt>
{
public:
  PromptCapRunner() : ActionRunner<capabilities2_msgs::action::CapabilityAction>()
  {
  }
};

}  // namespace prompt_bridge

// register the action runner
PLUGINLIB_EXPORT_CLASS(prompt_bridge::PromptRunner, capabilities2_runner::RunnerBase)
