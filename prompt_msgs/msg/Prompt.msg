# a single prompt message with a list of options
string prompt

# whether to flush the prompt
bool flush

# whether the prompt contains a file
bool contains_audio

# the type of the file, e.g., "audio/wav"
string file_type

# the audio buffer if the prompt contains a file
std_msgs/Int16MultiArray audio_buffer

# the options for the prompt, which can include model options, temperature, etc.
prompt_msgs/ModelOption[] options
