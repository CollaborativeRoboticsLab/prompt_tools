#pragma once

#include <functional>
#include <llm_prompt_provider_plugins/prompt_provider_base.hpp>
#include <memory>
#include <pluginlib/class_loader.hpp>
#include <prompt_msgs/action/plan.hpp>
#include <prompt_msgs/msg/prompt_history.hpp>
#include <prompt_msgs/msg/prompt_transaction.hpp>
#include <prompt_msgs/srv/prompt.hpp>
#include <prompt_schemes/scheme_base.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <thread>

namespace prompt_bridge
{
class PromptBridge : public rclcpp::Node
{
public:
  PromptBridge(const rclcpp::NodeOptions& options = rclcpp::NodeOptions())
    : Node("prompt_bridge", options), loop_hz_(1.0), frame_id_("agent"), transaction_limit_(10)
  {
    // loop rate
    loop_hz_ = this->declare_parameter("loop_rate", loop_hz_);

    // frame id
    frame_id_ = this->declare_parameter("frame_id", frame_id_);

    // number of transactions stored in history
    transaction_limit_ = this->declare_parameter("cached_transactions", 10);

    // offer service
    const bool offer_service = this->declare_parameter("offer_service", false);

    // create prompt provider from plugin class loader
    std::string plugin_name = this->declare_parameter("prompt_provider_plugin", "prompt_provider::"
                                                                                "DefaultPromptProvider");

    // create plugin loader
    RCLCPP_INFO(this->get_logger(), "Loading prompt provider plugin: '%s'", plugin_name.c_str());
    pluginlib::ClassLoader<prompt_provider::PromptProviderBase> loader("llm_prompt_provider_plugins", "prompt_provider:"
                                                                                                      ":PromptProviderB"
                                                                                                      "ase");

    prompt_provider_ = loader.createSharedInstance(plugin_name);

    // init provider
    // get a shared pointer to the node parameters interface
    prompt_provider_->init(this->get_node_parameters_interface(), this->get_node_logging_interface());

    // create prompt scheme from plugin class loader
    std::string scheme = this->declare_parameter("prompt_scheme_plugin", "prompt_scheme::DefaultScheme");

    // create plugin loader
    RCLCPP_INFO(this->get_logger(), "Loading prompt scheme plugin: '%s'", scheme.c_str());
    pluginlib::ClassLoader<prompt_schemes::SchemeBase> scheme_loader("prompt_schemes", "prompt_schemes::SchemeBase");

    scheme_ = scheme_loader.createSharedInstance(scheme);

    // init scheme
    scheme_->init(this->get_node_parameters_interface(), this->get_node_logging_interface());

    // history publisher
    prompt_history_pub_ = this->create_publisher<prompt_msgs::msg::PromptHistory>("~/history", 1);

    // optional service interface
    if (offer_service)
    {
      // create prompt service
      prompt_service_ = this->create_service<prompt_msgs::srv::Prompt>(
          "~/prompt", std::bind(&PromptBridge::prompt_service_cb, this, std::placeholders::_1, std::placeholders::_2));
    }

    // plan action server
    this->plan_action_server_ = rclcpp_action::create_server<prompt_msgs::action::Plan>(
        this, "~/plan", std::bind(&PromptBridge::handle_goal, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&PromptBridge::handle_cancel, this, std::placeholders::_1),
        std::bind(&PromptBridge::handle_accepted, this, std::placeholders::_1));

    // history publisher timer
    history_pub_timer_ = this->create_wall_timer(std::chrono::duration<double>(1.0),
                                                 std::bind(&PromptBridge::history_pub_timer_cb, this));

    RCLCPP_INFO(this->get_logger(), "PromptBridge initialized");
  }

  ~PromptBridge() = default;

  // service callback
  /**
   * @brief prompt service callback
   *
   * send and receive prompt messages to the prompt provider
   * this service is scheme-less and just transacts a prompt
   *
   * @param req
   * @param res
   * @return true
   * @return false
   */
  bool prompt_service_cb(const std::shared_ptr<prompt_msgs::srv::Prompt::Request> req,
                         std::shared_ptr<prompt_msgs::srv::Prompt::Response> res)
  {
    // print the prompt message
    RCLCPP_INFO(this->get_logger(), "Prompt: %s", req->prompt.prompt.c_str());

    // pre send time
    auto pre_send_time = this->now();

    // send the prompt message to the prompt provider
    try
    {
      prompt_provider::PromptProviderBase::PromptResponse result =
          prompt_provider_->sendPrompt(prompt_provider_->fromMsg(req->prompt));

      // set the response message
      res->response = prompt_provider_->toMsg(result);
    }
    catch (const prompt_provider::PromptProviderException& e)
    {
      RCLCPP_ERROR_STREAM(this->get_logger(), "Prompt provider failed to send prompt: " << e.what());
      return false;
    }

    // post send time
    auto post_send_time = this->now();

    // create the prompt transaction
    prompt_msgs::msg::PromptTransaction prompt_transaction = prompt_msgs::msg::PromptTransaction();
    prompt_transaction.prompt.header = std_msgs::msg::Header();
    prompt_transaction.prompt.header.stamp = pre_send_time;
    prompt_transaction.prompt.prompt = req->prompt;
    prompt_transaction.response.header = std_msgs::msg::Header();
    prompt_transaction.response.header.stamp = post_send_time;
    prompt_transaction.response.response = res->response;

    // update the prompt history
    prompt_history_.transactions.push_back(prompt_transaction);

    // remove old transactions
    while (prompt_history_.transactions.size() > transaction_limit_)
    {
      prompt_history_.transactions.erase(prompt_history_.transactions.begin());
    }

    return true;
  }

  // action callbacks
  rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID& uuid,
                                          std::shared_ptr<const prompt_msgs::action::Plan::Goal> goal)
  {
    // check if the goal is valid
    // prompt empty
    if (goal->goal.prompt.prompt.empty())
    {
      RCLCPP_ERROR(this->get_logger(), "goal prompt is empty");
      return rclcpp_action::GoalResponse::REJECT;
    }

    // check if the scheme is idle
    // FIXME: is this thread safe?
    if (scheme_->state() != prompt_schemes::SchemeBase::State::IDLE)
    {
      RCLCPP_ERROR(this->get_logger(), "scheme is not idle");
      return rclcpp_action::GoalResponse::REJECT;
    }

    // start the action
    RCLCPP_INFO(this->get_logger(), "prompt plan action accepted");
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  rclcpp_action::CancelResponse
  handle_cancel(const std::shared_ptr<rclcpp_action::ServerGoalHandle<prompt_msgs::action::Plan>> goal_handle)
  {
    // cancel the action
    RCLCPP_INFO(this->get_logger(), "prompt plan action canceled");
    (void)goal_handle;
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void handle_accepted(const std::shared_ptr<rclcpp_action::ServerGoalHandle<prompt_msgs::action::Plan>> goal_handle)
  {
    // start execution and detach
    std::thread{ std::bind(&PromptBridge::execute, this, std::placeholders::_1), goal_handle }.detach();
  }

  // execute action
  /**
   * @brief execute a plan action
   * handle prompt negotiation via a scheme and prompt provider
   * and publish feedback and result
   *
   * @param goal_handle
   */
  void execute(const std::shared_ptr<rclcpp_action::ServerGoalHandle<prompt_msgs::action::Plan>> goal_handle)
  {
    // set tick rate
    rclcpp::Rate rate(loop_hz_);  // 10 Hz

    // feedback and result objects
    auto feedback = std::make_shared<prompt_msgs::action::Plan::Feedback>();
    auto result = std::make_shared<prompt_msgs::action::Plan::Result>();

    // set the initial document string to the goal prompt
    scheme_->set_doc_str(goal_handle->get_goal()->goal.prompt.prompt);
    // set the prompt provider
    scheme_->set_prompt_provider(prompt_provider_);

    // tick the scheme to start (assuming that the scheme was idle)
    if (scheme_->state() == prompt_schemes::SchemeBase::State::IDLE)
    {
      scheme_->tick(scheme_->get_doc_str());
    }

    // while the scheme is not finished (idle) and the action is not canceled
    // and ros is not shutting down
    while (scheme_->state() != prompt_schemes::SchemeBase::State::IDLE && !goal_handle->is_canceling() && rclcpp::ok())
    {
      // tick the scheme
      // updating the scheme state machine
      // use the prompt provider set in the scheme
      scheme_->tick(scheme_->get_doc_str());

      // update feedback with the current state
      feedback->current_op = scheme_->op_string();
      goal_handle->publish_feedback(feedback);

      // sleep
      rate.sleep();
    }

    // if the action is canceled
    if (goal_handle->is_canceling())
    {
      result->summary = "plan action canceled";
      result->plan_docs.push_back(scheme_->get_doc_str());
      goal_handle->canceled(result);
      RCLCPP_INFO(this->get_logger(), "plan action canceled");
      return;
    }

    // if the scheme is finished
    if (scheme_->state() != prompt_schemes::SchemeBase::State::IDLE)
    {
      // set the result
      result->summary = goal_handle->get_goal()->goal.prompt.prompt;
      result->plan_docs.push_back(scheme_->get_doc_str());
      goal_handle->succeed(result);
      RCLCPP_INFO(this->get_logger(), "plan action succeeded");
    }

    return;
  }

private:
  // history pub timer callback
  void history_pub_timer_cb()
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

  // number of transactions stored in history
  unsigned int transaction_limit_;

  // prompt history
  prompt_msgs::msg::PromptHistory prompt_history_;

  // prompt provider
  std::shared_ptr<prompt_provider::PromptProviderBase> prompt_provider_;
  // prompt scheme
  std::shared_ptr<prompt_schemes::SchemeBase> scheme_;

  // ROS API
  // pubs
  rclcpp::Publisher<prompt_msgs::msg::PromptHistory>::SharedPtr prompt_history_pub_;
  // services
  rclcpp::Service<prompt_msgs::srv::Prompt>::SharedPtr prompt_service_;
  // actions
  rclcpp_action::Server<prompt_msgs::action::Plan>::SharedPtr plan_action_server_;
  // timers
  rclcpp::TimerBase::SharedPtr history_pub_timer_;
};

}  // namespace prompt_bridge
