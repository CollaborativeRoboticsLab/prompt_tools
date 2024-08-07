#pragma once

#include <string>
#include <vector>
// #include <behaviortree_cpp

namespace prompt_schemes {

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
class BTScheme {
 public:
  // is there xml in the text
  const bool is_parseable(const std::string& bt_xml) {
    // read string and find sufficient xml features to determine if it is
    // parseable or not
  }

  // parse string to xml
  void parse_xml(const std::string& bt_xml) {
    // parse string to xml

    // store in bt_xml_
  }

  // find the next part in the BT that is not clear enough for completion
  const std::string find_next_missing(const std::string& bt_xml);

 public:
  // bt xml artifact
  std::string bt_xml_;

  // registered actions to compare tree with
  std::vector<std::string> registered_actions_;

  // valid tree elements to compare tree with
  std::vector<std::string> valid_tree_elements_ = {
      "action", "condition", "sequence", "selector", "parallel"};
};

}  // namespace prompt_schemes
