#pragma once

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

  virtual void init(const rclcpp::Node::SharedPtr& node) = 0;

  // get current state
  const State state()
  {
    return state_;
  }

  // set document string
  void set_doc_str(const std::string& doc_str)
  {
    doc_str_ = doc_str;
  }

  // get document string
  const std::string get_doc_str()
  {
    return doc_str_;
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
  virtual const bool starting(const std::string& doc_str) = 0;
  // actions in collecting
  // collect request and list of available capabilities
  // these can be registered using plugins, capability provider, or other
  // explicit runtime registrations
  virtual const bool collecting(const std::string& doc_str) = 0;
  // actions in negotiating
  // negotiate document with prompt provider
  // this is the core of the prompt scheme
  virtual const bool negotiating(const std::string& doc_str) = 0;
  // actions in running
  // create and manage lifecycle
  // run the to completion or fail and return to negotiation
  virtual const bool running(const std::string& doc_str) = 0;
  // actions in idle
  // nothing to do in idle except to listen for request
  virtual const bool idle(const std::string& doc_str) = 0;

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

private:
  // current state
  State state_ = State::IDLE;

  // document string
  std::string doc_str_;
};

}  // namespace prompt_schemes
