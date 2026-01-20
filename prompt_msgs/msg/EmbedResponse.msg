# list of embeddings
prompt_msgs/Embedding[] embeddings

# indicates if the embedding generation was successful
bool success

# (optional) error message if the embedding generation failed
string error

# used model
string model

# token usage
int64 prompt_tokens
int64 total_tokens