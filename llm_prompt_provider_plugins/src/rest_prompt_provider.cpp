#include <llm_prompt_provider_plugins/rest_prompt_provider.hpp>
#include <pluginlib/class_list_macros.hpp>

namespace prompt_provider
{

void RestPromptProvider::init(
    rclcpp::node_interfaces::NodeParametersInterface::SharedPtr params,
    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr log)
{
  // set logger
  node_logging_interface_ptr_ = log;

  // default uri
  std::string default_uri = "https://localhost:8443/api/v1/prompt";

  // get uri from parameter server
  uri_ = params
             ->declare_parameter("RestPromptProvider.uri",
                                 rclcpp::ParameterValue(default_uri))
             .get<std::string>();

  // get method from parameter server
  method_ =
      params
          ->declare_parameter("RestPromptProvider.method", rclcpp::ParameterValue("POST"))
          .get<std::string>();

  // log
  RCLCPP_INFO(node_logging_interface_ptr_->get_logger(),
              "RestPromptProvider initialized with uri: %s, method: %s", uri_.c_str(),
              method_.c_str());
}

const PromptProviderBase::PromptResponse
RestPromptProvider::sendPrompt(const PromptRequest& req)
{
  // uri
  Poco::URI uri(uri_);

  // context without certificate verification
  Poco::Net::Context::Params params;
  params.verificationMode = Poco::Net::Context::VERIFY_NONE;
  Poco::Net::Context::Ptr context =
      new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, params);

  // create https session
  Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort(), context);

  // prepare request body
  Poco::JSON::Object body_json = RestPromptProvider::toJson(req);

  // calculate body length
  std::ostringstream body_stream;
  body_json.stringify(body_stream);

  // create request object
  Poco::Net::HTTPRequest request(method_, uri.getPath());
  // set headers
  request.setContentType("application/json");

  // send request
  std::ostream& os = session.sendRequest(request);

  // complete request body
  body_json.stringify(os);

  // get response
  Poco::Net::HTTPResponse response;
  std::istream& rs = session.receiveResponse(response);

  // parse response
  Poco::JSON::Parser parser;
  Poco::Dynamic::Var result = parser.parse(rs);
  Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

  // check for errors
  if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK)
  {
    throw PromptProviderException("HTTP Error: " + response.getReason());
  }

  // create response
  PromptProviderBase::PromptResponse res;
  res.response = object->get("response").toString();
  res.success = object->get("success").convert<bool>();
  res.accuracy = object->get("accuracy").convert<double>();
  res.confidence = object->get("confidence").convert<double>();
  res.risk = object->get("risk").convert<double>();

  return res;
}

void RestPromptProvider::handle_opts(const PromptProviderBase::PromptRequest& req)
{
}

}  // namespace prompt_provider

// plugin
PLUGINLIB_EXPORT_CLASS(prompt_provider::RestPromptProvider,
                       prompt_provider::PromptProviderBase)
