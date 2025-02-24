#pragma once
#include <string>
#include <vector>

namespace prompt_scheme
{
/**
 * @brief State
 *
 * An enum class representing the state of the prompt scheme. Each state
 * represents a different phase of the scheme's operation.
 *
 * `STARTING`:    The scheme is starting
 * `COLLECTING`:  The scheme is actively collecting data from the robot for the prompt provider.
 * `NEGOTIATING`: The scheme has collected enough information to be able to negotiate with the 
 *                prompt provider to finalise the an action document.
 * `RUNNING`:     The scheme has finalised the document and is running it on the robot.
 * `IDLE`:        The scheme is not currently active.
 */
enum class State
{
  STARTING,
  COLLECTING,
  NEGOTIATING,
  RUNNING,
  IDLE
};

}  // namespace prompt_scheme