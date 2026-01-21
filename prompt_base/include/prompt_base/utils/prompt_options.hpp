#pragma once

#include <exception>
#include <prompt_base/utils/structs.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>
#include <vector>

namespace prompt
{

/**
 * @brief Load prompt options from ROS2 parameters
 *
 * This function loads prompt options from the ROS2 parameter server for a given plugin.
 * It retrieves the option keys and their corresponding values and types, and constructs
 * a vector of PromptOption structs.
 *
 * @param node Shared pointer to the ROS2 node
 * @param plugin_name Name of the plugin for which to load options
 * @return std::vector<prompt::PromptOption> Vector of loaded prompt options
 */
std::vector<prompt::PromptOption> load_from_parameters(rclcpp::Node::SharedPtr node, std::string plugin_name)
{
  // init model options
  std::vector<std::string> model_option_keys;

  // declare option_keys only if not already declared
  if (node->has_parameter(plugin_name + ".option_keys"))
  {
    model_option_keys = node->get_parameter(plugin_name + ".option_keys").as_string_array();
  }
  else
  {
    model_option_keys =
        node->declare_parameter(plugin_name + ".option_keys", rclcpp::ParameterValue(std::vector<std::string>{}))
            .get<std::vector<std::string>>();
  }

  std::vector<prompt::PromptOption> options_;

  for (const auto& key : model_option_keys)
  {
    prompt::PromptOption opt;
    opt.key = key;

    std::string type_param_name = plugin_name + ".options." + key + ".type";
    if (node->has_parameter(type_param_name))
    {
      opt.type = node->get_parameter(type_param_name).as_string();
    }
    else
    {
      opt.type = node->declare_parameter(type_param_name, rclcpp::ParameterValue("")).get<std::string>();
    }

    // check if value is of type string
    if (opt.type == "string")
    {
      std::string value_param_name = plugin_name + ".options." + key + ".value";
      if (node->has_parameter(value_param_name))
      {
        opt.value = node->get_parameter(value_param_name).as_string();
      }
      else
      {
        opt.value = node->declare_parameter(value_param_name, rclcpp::ParameterValue("")).get<std::string>();
      }
    }

    // check if value is of type bool
    else if (opt.type == "bool")
    {
      std::string value_param_name = plugin_name + ".options." + key + ".value";
      bool value;
      if (node->has_parameter(value_param_name))
      {
        value = node->get_parameter(value_param_name).as_bool();
      }
      else
      {
        value = node->declare_parameter(value_param_name, rclcpp::ParameterValue(false)).get<bool>();
      }
      opt.value = (value) ? "true" : "false";
    }

    // check if value is of type bool
    else if (opt.type == "int")
    {
      std::string value_param_name = plugin_name + ".options." + key + ".value";
      int value;
      if (node->has_parameter(value_param_name))
      {
        value = node->get_parameter(value_param_name).as_int();
      }
      else
      {
        value = node->declare_parameter(value_param_name, rclcpp::ParameterValue(0)).get<int>();
      }
      opt.value = std::to_string(value);
    }

    // check if value is of type bool
    else if (opt.type == "double")
    {
      std::string value_param_name = plugin_name + ".options." + key + ".value";
      double value;
      if (node->has_parameter(value_param_name))
      {
        value = node->get_parameter(value_param_name).as_double();
      }
      else
      {
        value = node->declare_parameter(value_param_name, rclcpp::ParameterValue(0)).get<double>();
      }
      opt.value = std::to_string(value);
    }

    // check if type of value is not mentioned and take it as a string
    else
    {
      std::string value_param_name = plugin_name + ".options." + key + ".value";
      if (node->has_parameter(value_param_name))
      {
        opt.value = node->get_parameter(value_param_name).as_string();
      }
      else
      {
        opt.value = node->declare_parameter(value_param_name, rclcpp::ParameterValue("")).get<std::string>();
      }
    }

    options_.push_back(opt);
  }

  return options_;
}

/**
 * @brief Check and add required model options to the prompt options
 *
 * This method checks if the required model options are present in the prompt options.
 * If any required option is missing, it adds it to the prompt options.
 *
 * @param node Shared pointer to the ROS2 node
 * @param options The prompt options to check and modify
 * @param required_options The required model options to ensure are present
 */
void ensure_model_options(rclcpp::Node::SharedPtr node, std::vector<prompt::PromptOption>& options,
                         std::vector<prompt::PromptOption>& required_options)
{
  // check required options are available and if not add them
  for (const auto& required_opt : required_options)
  {
    bool found = false;
    for (const auto& opt : options)
    {
      if (opt.key == required_opt.key)
      {
        found = true;
        break;
      }
    }

    if (!found)
    {
      RCLCPP_WARN(node->get_logger(), "Model option '%s' not found, Adding '%s'", required_opt.key.c_str(),
                  required_opt.value.c_str());
      options.push_back(required_opt);
    }
  }
}
}  // namespace prompt