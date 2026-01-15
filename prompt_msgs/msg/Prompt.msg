# a single prompt message with a list of options
string prompt

# whether to use prompt caching
bool use_cache

# whether to flush the prompt cache
bool flush_cache

# whether to use chat mode or not
bool use_chat_mode

# what model family to use
string model_family

# the options for the prompt, which can include model options, temperature, etc.
prompt_msgs/ModelOption[] options
