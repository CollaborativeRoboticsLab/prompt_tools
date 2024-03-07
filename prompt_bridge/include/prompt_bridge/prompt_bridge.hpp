#pragma once

#include <ros/ros.h>
#include <prompt_msgs/PromptHistory.h>
#include <prompt_msgs/PromptTransaction.h>
#include <prompt_msgs/PromptRequest.h>
#include <prompt_msgs/PromptResponse.h>

#include <llm_prompt_provider_plugins/prompt_provider.hpp>

namespace prompt_bridge
{
class PromptBridge
{
public:
  PromptBridge(ros::NodeHandle& nh, ros::NodeHandle& pnh) : loop_rate_(1.0), frame_id_("agent")
  {
    // loop rate
    double loop_hz = pnh_.param("loop_rate", 1.0);
    loop_rate_ = ros::Rate(loop_hz);

    // frame id
    frame_id_ = pnh_.param("frame_id", frame_id_);

    // create prompt provider
    prompt_provider_ = std::make_shared<prompt_provider::PromptProvider>(nh, pnh);

    // history publisher
    prompt_history_pub_ = nh.advertise<prompt_msgs::PromptHistory>("~prompt_history", 1);

    // create prompt service
    prompt_service_ = nh.advertiseService("~prompt", &PromptBridge::prompt_service_cb, this);

    // prompt publisher timer
    history_pub_timer_ = nh.createTimer(loop_rate_.expectedCycleTime(), &PromptBridge::history_pub_timer_cb, this);
  }

  ~PromptBridge();

  // service callback
  bool prompt_service_cb(prompt_msgs::Prompt::Request& req, prompt_msgs::Prompt::Response& res)
  {
    // print the prompt message
    ROS_INFO_STREAM("Prompt: " << req.message);

    // pre send time
    ros::Time pre_send_time = ros::Time::now();

    // send the prompt message to the prompt provider
    try
    {
      prompt_provider::PromptResponse result = prompt_provider_->sendPrompt(prompt_provider_->fromMsg(req));
    }
    catch (const std::exception& e)
    {
      ROS_ERROR_STREAM("Prompt provider failed to send prompt: " << e.what());
      return false;
    }

    // set the response message
    res = prompt_provider_->toMsg(result);

    // post send time
    ros::Time post_send_time = ros::Time::now();

    // create the prompt transaction
    prompt_msgs::PromptTransaction prompt_transaction = prompt_msgs::PromptTransaction();
    prompt_transaction.prompt.header = std_msgs::Header();
    prompt_transaction.prompt.header.stamp = pre_send_time;
    prompt_transaction.prompt.prompt = req;
    prompt_transaction.response.header = std_msgs::Header();
    prompt_transaction.response.header.stamp = post_send_time;
    prompt_transaction.response.response = res;

    // update the prompt history
    prompt_history_.push_back(prompt_transaction);

    return true;
  }

  // history pub timer callback
  void history_pub_timer_cb(const ros::TimerEvent& event)
  {
    // publish the prompt history
    prompt_history_pub_.publish(prompt_history_);
  }

private:
  // ros params
  ros::Rate loop_rate_;   // loop rate
  std::string frame_id_;  // frame id

  // prompt history
  prompt_msgs::PromptHistory prompt_history_;

  // prompt provider
  std::make_shared<prompt_provider::PromptProvider> prompt_provider_;

  // ROS API
  // subs
  // pubs
  ros::Publisher prompt_history_pub_;
  // services
  ros::ServiceServer prompt_service_;
  // timers
  ros::Timer history_pub_timer_;
};

}  // namespace prompt_bridge
