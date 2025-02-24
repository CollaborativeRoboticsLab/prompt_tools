#pragma once

#include <functional>
#include <memory>
#include <pluginlib/class_loader.hpp>
#include <prompt_msgs/action/plan.hpp>
#include <prompt_msgs/action/prompt.hpp>
#include <prompt_msgs/msg/prompt_history.hpp>
#include <prompt_msgs/msg/prompt_transaction.hpp>
#include <prompt_msgs/srv/prompt.hpp>
#include <prompt_provider/prompt_provider_base.hpp>
#include <prompt_scheme/scheme_base.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <thread>

namespace prompt_planner
{
/**
 * @brief PromptPlanner
 *
 * This class provides a bridge between ROS resources and the prompt provider and scheme
 * It also provides a ROS action and that can be used to send and receive prompts
 *
 */
class PromptPlanner : public rclcpp::Node
{
  using PromptPlan = prompt_msgs::action::Plan;
  using PromptSrv = prompt_msgs::srv::Prompt;

public:
  PromptPlanner(const rclcpp::NodeOptions& options = rclcpp::NodeOptions())
    : Node("prompt_planner", options)
    , loop_hz_(1.0)
    , frame_id_("agent")
    , transaction_limit_(10)
    // , prompt_history_()
    , prompt_provider_loader_("prompt_provider", "prompt_provider::PromptProviderBase")
    , scheme_loader_("prompt_scheme", "prompt_schemes::SchemeBase")
  {
    // loop rate
    loop_hz_ = this->declare_parameter("loop_rate", loop_hz_);

    // frame id
    frame_id_ = this->declare_parameter("frame_id", frame_id_);

    // number of transactions stored in history
    transaction_limit_ = this->declare_parameter("cached_transactions", 10);

    /*************************************************************************
     * prompt provider plugin class loader and provider pointer
     ************************************************************************/

    // create prompt provider from plugin class loader
    std::string plugin_name = this->declare_parameter("prompt_provider", "prompt_provider::DefaultPromptProvider");

    RCLCPP_INFO(this->get_logger(), "Loading prompt provider plugin: '%s'", plugin_name.c_str());

    prompt_provider_ = prompt_provider_loader_.createSharedInstance(plugin_name);

    // init provider
    // get a shared pointer to the node parameters interface
    prompt_provider_->init(this->get_node_parameters_interface(), this->get_node_logging_interface());

    /*************************************************************************
     * prompt scheme plugin class loader and scheme pointer
     ************************************************************************/

    // create prompt scheme from plugin class loader
    std::string scheme = this->declare_parameter("prompt_scheme", "prompt_scheme::DefaultScheme");

    RCLCPP_INFO(this->get_logger(), "Loading prompt scheme plugin: '%s'", scheme.c_str());
    scheme_ = scheme_loader_.createSharedInstance(scheme);

    // init scheme
    scheme_->init(this->get_node_parameters_interface(), this->get_node_logging_interface());

    // set the prompt provider
    scheme_->set_prompt_provider(prompt_provider_);

    /*************************************************************************
     * prompt service and history ros interfaces
     ************************************************************************/

    // history publisher
    prompt_history_pub_ = this->create_publisher<prompt_msgs::msg::PromptHistory>("~/history/planner", 1);

    // plan action server
    this->plan_action_server_ = rclcpp_action::create_server<PromptPlan>(
        this, "~/plan", std::bind(&PromptPlanner::handle_plan_goal, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&PromptPlanner::handle_plan_cancel, this, std::placeholders::_1),
        std::bind(&PromptPlanner::handle_plan_accepted, this, std::placeholders::_1));

    // history publisher timer
    history_pub_timer_ =
        this->create_wall_timer(std::chrono::duration<double>(1.0), std::bind(&PromptPlanner::history_timer, this));

    RCLCPP_INFO(this->get_logger(), "PromptPlanner initialized");
  }

  ~PromptPlanner() = default;

  // action callbacks
  rclcpp_action::GoalResponse handle_plan_goal(const rclcpp_action::GoalUUID& uuid,
                                               std::shared_ptr<const PromptPlan::Goal> goal)
  {
    // check if the goal is valid
    // prompt empty
    if (goal->goal.prompt.prompt.empty())
    {
      RCLCPP_ERROR(this->get_logger(), "plan action goal prompt is empty");
      return rclcpp_action::GoalResponse::REJECT;
    }

    // check if the scheme is idle
    // FIXME: is this thread safe?
    if (scheme_->state() != prompt_scheme::State::IDLE)
    {
      RCLCPP_ERROR(this->get_logger(), "plan scheme is not idle");
      return rclcpp_action::GoalResponse::REJECT;
    }

    // start the action
    RCLCPP_DEBUG(this->get_logger(), "prompt plan action accepted");
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  rclcpp_action::CancelResponse
  handle_plan_cancel(const std::shared_ptr<rclcpp_action::ServerGoalHandle<PromptPlan>> goal_handle)
  {
    // cancel the action
    RCLCPP_WARN(this->get_logger(), "prompt plan action canceled");
    (void)goal_handle;
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void handle_plan_accepted(const std::shared_ptr<rclcpp_action::ServerGoalHandle<PromptPlan>> goal_handle)
  {
    // start execution and detach
    std::thread{ std::bind(&PromptPlanner::plan_execute, this, std::placeholders::_1), goal_handle }.detach();
  }

  // execute plan action
  /**
   * @brief execute a plan action
   * handle prompt negotiation via a scheme and prompt provider
   * and publish feedback and result
   *
   * @param goal_handle
   */
  void plan_execute(const std::shared_ptr<rclcpp_action::ServerGoalHandle<PromptPlan>> goal_handle)
  {
    // set tick rate
    rclcpp::Rate rate(loop_hz_);  // 10 Hz

    // feedback and result objects
    auto feedback = std::make_shared<PromptPlan::Feedback>();
    auto result = std::make_shared<PromptPlan::Result>();

    // log
    RCLCPP_INFO(this->get_logger(), "prompt-plan goal: %s", goal_handle->get_goal()->goal.prompt.prompt.c_str());

    // set the initial document string to the goal prompt
    scheme_->set_prompt(goal_handle->get_goal()->goal.prompt.prompt);

    // tick the scheme to start (assuming that the scheme was idle)
    if (scheme_->state() == prompt_scheme::State::IDLE)
    {
      scheme_->tick(scheme_->get_prompt());
    }

    // while the scheme is not finished (idle) and the action is not canceled
    // and ros is not shutting down
    while (scheme_->state() != prompt_scheme::State::IDLE && !goal_handle->is_canceling() && rclcpp::ok())
    {
      // tick the scheme
      // updating the scheme state machine
      // use the prompt provider set in the scheme
      scheme_->tick(scheme_->get_prompt());

      // update feedback with the current state
      feedback->current_op = scheme_->get_state_string();
      goal_handle->publish_feedback(feedback);

      RCLCPP_WARN(this->get_logger(), "current scheme: %s", scheme_->get_prompt().c_str());
      RCLCPP_WARN(this->get_logger(), "current op: %s", scheme_->get_state_string().c_str());

      // sleep
      rate.sleep();
    }

    // if the action is canceled
    if (goal_handle->is_canceling())
    {
      result->summary = "plan action canceled";
      result->plan_docs.push_back(scheme_->get_prompt());
      goal_handle->canceled(result);
      RCLCPP_INFO(this->get_logger(), "plan action canceled");
      return;
    }

    // if the scheme is finished
    if (scheme_->state() != prompt_scheme::State::IDLE)
    {
      // set the result
      result->summary = goal_handle->get_goal()->goal.prompt.prompt;
      result->plan_docs.push_back(scheme_->get_prompt());
      goal_handle->succeed(result);
      RCLCPP_INFO(this->get_logger(), "plan action succeeded");
    }

    return;
  }

private:
  // history pub timer callback
  void history_timer()
  {
    // publish the prompt history
    prompt_history_.header.frame_id = frame_id_;
    prompt_history_.header.stamp = this->now();
    prompt_history_pub_->publish(prompt_history_);
  }

private:
  // ros params
  double loop_hz_;        // loop rate
  std::string frame_id_;  // frame id
  bool use_scheme_;       // use scheme or not

  // number of transactions stored in history
  unsigned int transaction_limit_;

  // prompt history
  prompt_msgs::msg::PromptHistory prompt_history_;

  // loaders
  pluginlib::ClassLoader<prompt_provider::PromptProviderBase> prompt_provider_loader_;
  pluginlib::ClassLoader<prompt_scheme::SchemeBase> scheme_loader_;

  // prompt provider
  std::shared_ptr<prompt_provider::PromptProviderBase> prompt_provider_;

  // prompt scheme
  std::shared_ptr<prompt_scheme::SchemeBase> scheme_;

  // ROS API
  // pubs
  rclcpp::Publisher<prompt_msgs::msg::PromptHistory>::SharedPtr prompt_history_pub_;

  // actions
  rclcpp_action::Server<PromptPlan>::SharedPtr plan_action_server_;

  // timers
  rclcpp::TimerBase::SharedPtr history_pub_timer_;
};

}  // namespace prompt_planner
