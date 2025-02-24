#pragma once

#include <prompt_provider/prompt_provider_base.hpp>
#include <prompt_scheme/tools/structs.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>

namespace prompt_scheme
{

/**
 * @brief SchemeBase
 *
 * Base class for prompt schemes that are used in the prompt bridge
 * provides a set of rules to augment the prompting.
 */
class SchemeBase
{
public:
  SchemeBase() = default;
  virtual ~SchemeBase() = default;

  virtual void init(rclcpp::node_interfaces::NodeParametersInterface::SharedPtr params,
                    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr log) = 0;

  virtual void sendPrompt(const prompt::PromptRequest &req);

  void set_prompt(const std::string& prompt)
  {
    prompt_ = prompt;
  }

  // set prompt provider
  void set_prompt_provider(const std::shared_ptr<prompt_provider::PromptProviderBase>& prompt_provider)
  {
    prompt_provider_ = prompt_provider;
  }

  const std::string& get_prompt()
  {
    return prompt_;
  }

  // get current state
  const State& state()
  {
    return state_;
  }

  const std::string get_state_string()
  {
    switch (state_)
    {
      case State::STARTING:
        return "STARTING";
      case State::COLLECTING:
        return "COLLECTING";
      case State::NEGOTIATING:
        return "NEGOTIATING";
      case State::RUNNING:
        return "RUNNING";
      case State::IDLE:
        return "IDLE";
    }

    return "UNDEFINED";
  }

  // transition states
  void start()
  {
    state_ = State::STARTING;
  }

  void collect()
  {
    state_ = State::COLLECTING;
  }

  void negotiate()
  {
    state_ = State::NEGOTIATING;
  }

  void run()
  {
    state_ = State::RUNNING;
  }

  void idle()
  {
    state_ = State::IDLE;
  }

  // function to iterate through state actions and transitions
  void tick(const std::string prompt)
  {
    bool result = false;

    switch (state_)
    {
      case State::STARTING:
        result = starting(prompt);
        break;
      case State::COLLECTING:
        result = collecting(prompt);
        break;
      case State::NEGOTIATING:
        result = negotiating(prompt);
        break;
      case State::RUNNING:
        result = running(prompt);
        break;
      case State::IDLE:
        result = idling(prompt);
        break;
    }

    if (result)
      return next();
  }


protected:
  // virtual functions for each state
  // actions in starting
  // was started by external request
  virtual bool starting(const std::string& prompt) = 0;

  // actions in collecting
  // collect request and list of available capabilities
  // these can be registered using plugins, capability provider, or other
  // explicit runtime registrations
  virtual bool collecting(const std::string& prompt) = 0;

  // actions in negotiating
  // negotiate document with prompt provider
  // this is the core of the prompt scheme
  virtual bool negotiating(const std::string& prompt) = 0;

  // actions in running
  // create and manage lifecycle
  // run the to completion or fail and return to negotiation
  virtual bool running(const std::string& prompt) = 0;

  // actions in idle
  // nothing to do in idle except to listen for request
  virtual bool idling(const std::string& prompt)
  {
    // skip this state
    // essentially if tick is called then the scheme starts
    return true;
  }

  // transition to next state
  void next()
  {
    switch (state_)
    {
      case State::STARTING:
        collect();
        break;
      case State::COLLECTING:
        negotiate();
        break;
      case State::NEGOTIATING:
        run();
        break;
      case State::RUNNING:
        idle();
        break;
      case State::IDLE:
        start();
        break;
    }
  }

  void retry()
  {
    switch (state_)
    {
      case State::RUNNING:
        negotiate();
        break;
    }
  }
  
  // internal document string for negotiation updates
  std::string prompt_;

  // current state
  State state_ = State::IDLE;

  // logging interface
  rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr logging_;

  // current prompt provider
  std::shared_ptr<prompt_provider::PromptProviderBase> prompt_provider_;
};

}  // namespace prompt_scheme
