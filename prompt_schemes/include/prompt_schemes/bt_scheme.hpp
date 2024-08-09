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

  virtual void init(const rclcpp::Node::SharedPtr& node)
  {
  }

  // register robot action
  const void register_action(const std::string& action)
  {
  }

public:
  // parse xml to BT
  const void parse_xml_to_bt(const std::string& bt_xml)
  {
    // parse xml string to BT
    factory_.registerBehaviorTreeFromText(bt_xml);
  }

  // find the next part in the BT that is not clear enough for completion
  const std::string find_next_missing(const std::string& bt_xml);

protected:
  // implement the base class state functions
  virtual const bool starting(const std::string& doc_str)
  {
    // skip this state
    return true;
  }

  virtual const bool collecting(const std::string& doc_str)
  {
    // move to next state as the registered actions should be present
    return true;
  }

  virtual const bool negotiating(const std::string& doc_str)
  {
    // try passing the xml
    try
    {
      // parse the xml
      tinyxml2::XMLDocument doc = XMLScrubber::parse_xml(doc_str);

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

  virtual const bool running(const std::string& doc_str)
  {
    // scrub the xml
    std::string scrubbed_xml = XMLScrubber::scrub_xml(doc_str);

    // parse the xml to BT
    parse_xml_to_bt(scrubbed_xml);

    // skip this state
    return true;
  }

  virtual const bool idle(const std::string& doc_str)
  {
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
