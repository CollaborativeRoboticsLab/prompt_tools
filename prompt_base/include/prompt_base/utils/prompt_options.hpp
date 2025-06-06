#pragma once

#include <exception>
#include <prompt_base/utils/structs.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>
#include <vector>

namespace prompt
{
std::vector<prompt::PromptOption>
load_from_paramters(rclcpp::Node::SharedPtr node, std::string class_name)
{
  // init model options
  std::vector<std::string> model_option_keys =
      node->declare_parameter(class_name + ".prompt_option_keys", rclcpp::ParameterValue(std::vector<std::string>{}))
          .get<std::vector<std::string>>();

  std::vector<prompt::PromptOption> prompt_options_;

  for (const auto& key : model_option_keys)
  {
    prompt::PromptOption opt;

    opt.key = key;
    opt.type = node->declare_parameter(class_name + ".prompt_options." + key + ".type", rclcpp::ParameterValue(""))
                   .get<std::string>();

    // check if value is of type string
    if (opt.type == "string")
    {
      opt.value =
          node->declare_parameter(class_name + ".prompt_options." + key + ".value", rclcpp::ParameterValue(""))
              .get<std::string>();
    }

    // check if value is of type bool
    else if (opt.type == "bool")
    {
      bool value =
          node->declare_parameter(class_name + ".prompt_options." + key + ".value", rclcpp::ParameterValue(false))
              .get<bool>();
      opt.value = (value) ? "true" : "false";
    }

    // check if value is of type bool
    else if (opt.type == "int")
    {
      int value = node->declare_parameter(class_name + ".prompt_options." + key + ".value", rclcpp::ParameterValue(0))
                      .get<int>();
      opt.value = std::to_string(value);
    }

    // check if value is of type bool
    else if (opt.type == "double")
    {
      double value =
          node->declare_parameter(class_name + ".prompt_options." + key + ".value", rclcpp::ParameterValue(0))
              .get<double>();
      opt.value = std::to_string(value);
    }

    // check if type of value is not mentioned and take it as a string
    else
    {
      opt.value =
          node->declare_parameter(class_name + ".prompt_options." + key + ".value", rclcpp::ParameterValue(""))
              .get<std::string>();
    }

    prompt_options_.push_back(opt);
  }

  return prompt_options_;
}
}  // namespace prompt