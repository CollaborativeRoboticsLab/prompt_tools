#pragma once

#include <exception>
#include <prompt_base/utils/structs.hpp>
#include <rclcpp/rclcpp.hpp>
#include <string>
#include <vector>

namespace prompt
{

std::map<std::string, std::map<std::string, std::string>> load_model_families(rclcpp::Node::SharedPtr node)
{
  // init model families
  std::vector<std::string> model_family_keys =
      node->declare_parameter("model_family_names", rclcpp::ParameterValue(std::vector<std::string>{}))
          .get<std::vector<std::string>>();

  std::map<std::string, std::map<std::string, std::string>> model_families_;

  for (const auto& family_key : model_family_keys)
  {
    std::map<std::string, std::string> family_options;
    std::string single_plugin, chat_plugin;

    single_plugin = node->declare_parameter("model_family_plugins." + family_key + ".single_prompt_plugin",
                                            rclcpp::ParameterValue(""))
                        .get<std::string>();

    chat_plugin = node->declare_parameter("model_family_plugins." + family_key + ".chat_prompt_plugin",
                                          rclcpp::ParameterValue(""))
                      .get<std::string>();

    family_options["single"] = single_plugin;
    family_options["chat"] = chat_plugin;

    model_families_[family_key] = family_options;
  }

  return model_families_;
}

}  // namespace prompt