#pragma once

#include <behaviortree_cpp/bt_factory.h>
#include <tinyxml2.h>

#include <prompt_schemes/scheme_base.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace prompt_schemes
{

/**
 * @brief XMLScrubber
 *
 * text to xml support for text that may contain errors and additional non-xml
 * data
 *
 */
class XMLScrubber
{
public:
  XMLScrubber() = default;
  ~XMLScrubber() = default;

  // static functions for document scrubbing
  static const std::string scrub_xml(const std::string& xml_str)
  {
    return XMLScrubber::remove_outer_data(XMLScrubber::fix_xml_syntax(xml_str));
  }

  // remove outer non-xml data
  static const std::string remove_outer_data(const std::string& xml_str)
  {
    // find first root xml element
    std::string::size_type start = xml_str.find("<root");

    // find final root xml element
    std::string::size_type end = xml_str.find("</root>");

    // and return remaining xml
    return xml_str.substr(start, end - start);
  }

  // find and repair xml syntax errors
  static const std::string fix_xml_syntax(const std::string& xml_str)
  {
    // TODO: implement xml syntax error fixing
    return xml_str;
  }

  // is there xml in the text
  static const bool is_parseable(const std::string& xml_str)
  {
    // read string and find sufficient xml features to determine if it is
    // parseable or not
    return (xml_str.find("<root>") != std::string::npos);
  }

  // parse string to xml
  static const tinyxml2::XMLDocument parse_xml(const std::string& xml_str)
  {
    // is the string parseable
    if (!XMLScrubber::is_parseable(xml_str))
    {
      throw std::runtime_error("document does not contain parseable XML");
    }

    // scrub xml
    std::string scrubbed_xml = XMLScrubber::scrub_xml(xml_str);

    // parse string to xml
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError eResult = doc.Parse(xml_str.c_str());

    // check for parse errors
    if (eResult != tinyxml2::XML_SUCCESS)
    {
      throw std::runtime_error("failed to parse XML");
    }

    // return doc
    return doc.ToDocument();
  }
};

/**
 * @brief BTScheme
 *
 * prompt scheme for the behaviour tree xml format
 * a BT is created by conversation between prompt bridge and language model
 * once a BT document is finalised, it is actioned onboard the robot
 * this scheme is used when a robot is requisitioned for a complex goal
 * and a language model is used to guide the robot to achieve that goal
 *
 */
class BTScheme : public SchemeBase
{
public:
  BTScheme() = default;

  virtual void init(rclcpp::node_interfaces::NodeParametersInterface::SharedPtr params,
                    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr log)
  {
    // rclcpp::ParameterValue t;
    // t.get<std::vector<rclcpp::Parameter>>();
    // init model options
    std::vector<std::string> model_option_keys =
        params->declare_parameter("BTScheme.prompt_option_keys", rclcpp::ParameterValue(std::vector<std::string>{}))
            .get<std::vector<std::string>>();
    for (const auto& key : model_option_keys)
    {
      prompt_provider::PromptProviderBase::PromptOption opt;

      opt.key = key;
      opt.value = params->declare_parameter("BTScheme.prompt_options." + key + ".value", rclcpp::ParameterValue(""))
                      .get<std::string>();
      opt.type = params->declare_parameter("BTScheme.prompt_options." + key + ".type", rclcpp::ParameterValue(""))
                     .get<std::string>();

      prompt_options_.push_back(opt);
    }

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
        params->declare_parameter("BTScheme.system_role", rclcpp::ParameterValue(prompt_template_.system_role))
            .get<std::string>();
    prompt_template_.objective_brief =
        params->declare_parameter("BTScheme.objective_brief", rclcpp::ParameterValue(prompt_template_.objective_brief))
            .get<std::string>();
    prompt_template_.result_format_guide =
        params
            ->declare_parameter("BTScheme.result_format_guide",
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
        params->declare_parameter("BTScheme.additional_actions", rclcpp::ParameterValue(std::vector<std::string>()))
            .get<std::vector<std::string>>();
    for (const std::string& action : param_specified_actions)
    {
      prompt_template_.available_actions.push_back(action);
    }

    // TODO: register actions
  }

  // register robot action
  void register_action(const std::string& action)
  {
    // TODO: add action to factory
  }

public:
  // parse xml to BT
  void parse_xml_to_bt(const std::string& bt_xml)
  {
    // parse xml string to BT
    factory_.registerBehaviorTreeFromText(bt_xml);
  }

  // find the next part in the BT that is not clear enough for completion
  const std::string find_next_missing(const std::string& bt_xml);

protected:
  // implement the base class state functions
  virtual bool starting(const std::string& doc_str)
  {
    // set the next prompt to the document string
    start_with_doc_str(doc_str);
    // skip this state otherwise
    return true;
  }

  virtual bool collecting(const std::string& doc_str)
  {
    // move to next state as the registered actions should be present
    return true;
  }

  virtual bool negotiating(const std::string& doc_str)
  {
    // send next prompt
    set_doc_str(send_next_prompt());

    // try passing the xml
    try
    {
      // parse the supposed bt xml
      parse_xml_to_bt(doc_str);

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

  virtual bool running(const std::string& doc_str)
  {
    // scrub the xml
    std::string scrubbed_xml = XMLScrubber::scrub_xml(doc_str);

    // parse the xml to BT
    parse_xml_to_bt(scrubbed_xml);

    // skip this state
    return true;
  }

private:
  // bt factory
  BT::BehaviorTreeFactory factory_;

  // registered actions to compare tree with
  std::vector<std::string> registered_actions_;
};

}  // namespace prompt_schemes
