#pragma once
#include <string>
#include <tinyxml2.h>
#include <stdexcept>

namespace prompt
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

}