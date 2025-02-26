#pragma once

#include <behaviortree_cpp/bt_factory.h>

#include <prompt_scheme/scheme_base.hpp>
#include <prompt_scheme/tools/structs.hpp>
#include <prompt_scheme/tools/xml_scrubber.hpp>
#include <prompt_utils/structs.hpp>
#include <prompt_utils/prompt_options.hpp>
#include <string>
#include <vector>

namespace prompt_scheme
{

struct prompt_template_t
{
  std::string system_role;
  std::string objective_brief;
  std::vector<std::string> available_actions;
  std::string result_format_guide;
  std::string user_query;

  const std::string generate()
  {
    std::string prompt = system_role + "\n" + objective_brief + "\n";

    // add action list
    for (const auto& action : available_actions)
    {
      prompt += action + "\n";
    }

    // add result format guide
    prompt += result_format_guide + "\n";

    // add user query
    prompt += "the current user query is: " + user_query;

    return prompt;
  }
};

/**
 * @brief BTPlannerScheme
 *
 * prompt scheme for the behaviour tree xml format
 * a BT is created by conversation between prompt bridge and language model
 * once a BT document is finalised, it is actioned onboard the robot
 * this scheme is used when a robot is requisitioned for a complex goal
 * and a language model is used to guide the robot to achieve that goal
 *
 */
class BTPlannerScheme : public SchemeBase
{
public:
  BTPlannerScheme() = default;

  virtual void init(rclcpp::node_interfaces::NodeParametersInterface::SharedPtr params,
                    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr log)
  {
    logging_ = log;

    // rclcpp::ParameterValue t;
    // t.get<std::vector<rclcpp::Parameter>>();
    // init model options
    prompt_options_ = prompt::load_from_paramters(params, "BTPlannerScheme");

    // fill in the prompt template with default scheme
    prompt_template_.system_role = "you are an AI designed to plan complex tasks as part of a cooperative supervisory "
                                   "control system for a robot";

    prompt_template_.objective_brief = "you are responsible for generating a structured plan for the robot to achieve "
                                       "a complex goal provided by the user";

    prompt_template_.result_format_guide = "The generated plan should only contain a behaviour tree in xml format "
                                           "using available actions provided with this prompt, or stored in a "
                                           "document base";

    // update the prompt template with any parameters if available
    prompt_template_.system_role =
        params->declare_parameter("BTPlannerScheme.system_role", rclcpp::ParameterValue(prompt_template_.system_role))
            .get<std::string>();
    prompt_template_.objective_brief = params
                                           ->declare_parameter("BTPlannerScheme.objective_brief",
                                                               rclcpp::ParameterValue(prompt_template_.objective_brief))
                                           .get<std::string>();
    prompt_template_.result_format_guide =
        params
            ->declare_parameter("BTPlannerScheme.result_format_guide",
                                rclcpp::ParameterValue(prompt_template_.result_format_guide))
            .get<std::string>();

    // add fallback and placeholder actions
    prompt_template_.available_actions.push_back("fallback: an action that can be used to recover from errors. This "
                                                 "action will be rendered by the most relevant available action. For "
                                                 "example, in a 2d navigation problem it might be to stop, clear the "
                                                 "costmap, and spin around. Use this fallback action to properly "
                                                 "structure uncertain actions in the behaviour tree");

    prompt_template_.available_actions.push_back("placeholder: this action will be sought from the prompt provider at "
                                                 "a later time. This can be used as a placeholder for future actions, "
                                                 "or for an insertion point for a sub tree in the behaviour tree");

    // add other actions from parameters
    std::vector<std::string> param_specified_actions =
        params
            ->declare_parameter("BTPlannerScheme.additional_actions",
                                rclcpp::ParameterValue(std::vector<std::string>()))
            .get<std::vector<std::string>>();

    for (const std::string& action : param_specified_actions)
    {
      prompt_template_.available_actions.push_back(action);
    }

    // TODO: register actions
  }

  virtual prompt::PromptResponse processPrompt(const prompt::PromptRequest& req) override
  {
    prompt_ = req.prompt;
    prompt::PromptResponse response;
    return response;
  }

private:
  // register robot action
  void register_action(const std::string& action)
  {
    // TODO: add action to factory
  }

  // parse xml to BT
  void parse_xml_to_bt(const std::string& bt_xml)
  {
    // parse xml string to BT
    factory_.registerBehaviorTreeFromText(bt_xml);
  }

  // find the next part in the BT that is not clear enough for completion
  const std::string find_next_missing(const std::string& bt_xml);

protected:
  /**
   * @brief transition to previous state only really useful in the running state function
   *  to allow quick renegotiation
   */

  // set next prompt to doc string
  void start_with_prompt(const std::string& prompt)
  {
    next_prompt_ = prompt;
  }

  // implement the base class state functions
  virtual bool starting(const std::string& prompt)
  {
    // set the next prompt to the document string
    start_with_prompt(prompt);

    // skip this state otherwise
    return true;
  }

  virtual bool collecting(const std::string& prompt)
  {
    // move to next state as the registered actions should be present
    return true;
  }

  virtual bool negotiating(const std::string& prompt)
  {
    // send next prompt
    prompt_ = send_next_prompt();

    // try passing the xml
    try
    {
      // parse the supposed bt xml
      parse_xml_to_bt(prompt);

      // compare the xml to the registered actions
      // and request more information if the actions are not found
    }
    catch (const std::runtime_error& e)
    {
      // return false if the xml is not parseable
      return false;
    }

    return true;
  }

  virtual bool running(const std::string& prompt)
  {
    // scrub the xml
    std::string scrubbed_xml = prompt::XMLScrubber::scrub_xml(prompt);

    // parse the xml to BT
    parse_xml_to_bt(scrubbed_xml);

    // skip this state
    return true;
  }

  // send next prompt
  const std::string send_next_prompt()
  {
    if (prompt_provider_ != nullptr)
    {
      prompt_template_.user_query = next_prompt_;

      prompt::PromptRequest req;
      req.prompt = prompt_template_.generate();
      req.options = prompt_options_;

      const prompt::PromptResponse res = prompt_provider_->sendPrompt(req);

      return res.response;
    }

    return prompt_;
  }

private:
  // internal prompt for next negotiation
  std::string next_prompt_;

  // bt factory
  BT::BehaviorTreeFactory factory_;

  // registered actions to compare tree with
  std::vector<std::string> registered_actions_;

  // scheme prompt template
  prompt_scheme::prompt_template_t prompt_template_;

  // model options
  std::vector<prompt::PromptOption> prompt_options_;
};

}  // namespace prompt_scheme
