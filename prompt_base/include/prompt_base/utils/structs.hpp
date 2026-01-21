#pragma once
#include <string>
#include <vector>

namespace prompt
{
// prompt option
struct PromptOption
{
  std::string key;
  std::string value;
  std::string type;
};

// Prompt Dialog struct
struct PromptDialogue
{
  std::string role;
  std::string content;
};

// prompt provider request
struct PromptRequest
{
  std::string prompt;
  bool use_cache;
  bool flush_cache;
  bool use_chat_mode;
  std::string model_family;
  std::vector<PromptOption> options;
};

// prompt provider response
struct PromptResponse
{
  std::string response;
  bool buffered;
  std::vector<PromptOption> options;
  bool success{ false };
  double accuracy{ 0.0 };
  double confidence{ 0.0 };
  double risk{ 0.0 };
};

struct EmbedRequest
{
  std::string text;
  std::string model_family;
  std::vector<PromptOption> options;
};

struct Embedding
{
  std::string base64_embedding;
  std::vector<float> float_embedding;
  bool is_float{ true };
  int index{ -1 };
};

struct EmbedResponse
{
  std::vector<Embedding> embeddings;
  bool success{ false };
  std::string error;
  std::string model;
  int prompt_tokens{ 0 };
  int total_tokens{ 0 };
};

struct TokenRequest
{
  std::string text;
  std::vector<int> tokens;
  bool encode{ true };
  std::string model_family;
  std::vector<PromptOption> options;
};

struct TokenResponse
{
  std::vector<int> tokens;
  std::string text;
  bool success{ false };
  std::string error;
};


}  // namespace prompt