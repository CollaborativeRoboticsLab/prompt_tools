#pragma once

#include <llm_prompt_provider_plugins/prompt_provider_base.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>

namespace prompt_schemes
{

/**
 * @brief SchemeBase
 *
 * Base class for prompt schemes that are used in the prompt bridge
 * the class keeps track of the state of the scheme
 * and a record of the document string that is being negotiated
 * with the prompt provider
 *
 */
class SchemeBase
{
public:
  // scheme state type
  /**
   * @brief State
   *
   * An enum class representing the state of the prompt scheme. Each state
   * represents a different phase of the scheme's operation.
   *
   * * `STARTING`: The scheme is starting.
   * * `COLLECTING`: The scheme is actively collecting data from the robot for
   * the prompt provider.
   * * `NEGOTIATING`: The scheme has collected enough information to be able to
   * negotiate with the prompt provider to finalise the an action document.
   * * `RUNNING`: The scheme has finalised the document and is running it on
   *               the robot.
   * * `IDLE`: The scheme is not currently active.
   */
  enum class State
  {
    STARTING,
    COLLECTING,
    NEGOTIATING,
    RUNNING,
    IDLE
  };

public:
  SchemeBase() = default;
  virtual ~SchemeBase() = default;

  virtual void init(rclcpp::node_interfaces::NodeParametersInterface::SharedPtr params,
                    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr log) = 0;

  // get current state
  const State& state()
  {
    return state_;
  }

  const std::string op_string()
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

  // set document string
  void set_doc_str(const std::string& doc_str)
  {
    doc_str_ = doc_str;
  }

  // get document string
  const std::string& get_doc_str()
  {
    return doc_str_;
  }

  // set prompt provider
  void set_prompt_provider(
      const std::shared_ptr<prompt_provider::PromptProviderBase>& prompt_provider)
  {
    prompt_provider_ = prompt_provider;
  }

  // function to iterate through state actions and transitions
  void tick(const std::string doc_str)
  {
    bool res = false;

    switch (state_)
    {
      case State::STARTING:
        res = starting(doc_str);
        break;
      case State::COLLECTING:
        res = collecting(doc_str);
        break;
      case State::NEGOTIATING:
        res = negotiating(doc_str);
        break;
      case State::RUNNING:
        res = running(doc_str);
        break;
      case State::IDLE:
        res = idle(doc_str);
        break;
    }

    if (res)
      return next();
  }

protected:
  // virtual functions for each state
  // actions in starting
  // was started by external request
  virtual bool starting(const std::string& doc_str) = 0;
  // actions in collecting
  // collect request and list of available capabilities
  // these can be registered using plugins, capability provider, or other
  // explicit runtime registrations
  virtual bool collecting(const std::string& doc_str) = 0;
  // actions in negotiating
  // negotiate document with prompt provider
  // this is the core of the prompt scheme
  virtual bool negotiating(const std::string& doc_str) = 0;
  // actions in running
  // create and manage lifecycle
  // run the to completion or fail and return to negotiation
  virtual bool running(const std::string& doc_str) = 0;
  // actions in idle
  // nothing to do in idle except to listen for request
  virtual bool idle(const std::string& doc_str)
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

  // transition to previous state
  // only really useful in the running state function
  // to allow quick renegotiation
  void retry()
  {
    switch (state_)
    {
      case State::RUNNING:
        negotiate();
        break;
    }
  }

  // set next prompt to doc string
  void start_with_doc_str(const std::string& doc_str)
  {
    next_prompt_ = doc_str_;
  }

private:
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

protected:
  rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr node_logging_interface_ptr_;

private:
  // current state
  State state_ = State::IDLE;

  // internal document string for negotiation updates
  std::string doc_str_;

  // internal prompt for next negotiation
  std::string next_prompt_;

  // current prompt provider
  std::shared_ptr<prompt_provider::PromptProviderBase> prompt_provider_;
};

}  // namespace prompt_schemes
