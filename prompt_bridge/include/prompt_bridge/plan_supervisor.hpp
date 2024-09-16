#pragma once

#include <rclcpp/rclcpp.hpp>

namespace prompt_bridge
{
/**
 * @brief PlanSupervisor provides an action interface to supervise a plan
 *
 */
class PlanSupervisor : public rclcpp::Node
{
public:
  PlanSupervisor(/* args */);
  ~PlanSupervisor();

private:
};

}  // namespace prompt_bridge
