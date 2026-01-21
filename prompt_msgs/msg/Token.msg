# string to be converted into Tokens
string text

# list of tokens to be converted into text
int32[] tokens

# whether to encode (if false will be decoded)
bool encode

# model family to be used
string model_family

# the options for the prompt, which can include model etc.
prompt_msgs/ModelOption[] options