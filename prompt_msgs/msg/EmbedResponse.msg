# Embedding response if encoding_format is "float"
float32[] float_embedding

# Embedding response if encoding_format is "base64"
string base64_embedding

# (optional) provided embedding format
prompt_msgs/EmbedFormat format

# indicates if the embedding generation was successful
bool success

# (optional) error message if the embedding generation failed
string error