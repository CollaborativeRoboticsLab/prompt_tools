#pragma once
#include <exception>
#include <string>

namespace prompt
{

    // provider exception class
class PromptProviderException : public std::exception
{
public:
  PromptProviderException(const std::string& msg) : msg_(msg)
  {
  }

  virtual const char* what() const noexcept override
  {
    return msg_.c_str();
  }

private:
  std::string msg_;
};

}