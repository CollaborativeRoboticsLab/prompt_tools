# llm proxy

Local proxy for prompt tools to use to access LLMs.

Helps with integration as it can be used as a known endpoint for all LLMs simplifying the configuration of the prompt tools.

based on [Ollama](https://github.com/ollama/ollama)

## Usage

```bash
# start the proxy using docker
# make sure to use these containers on a capable machine
docker-compose up
```

## Configuration

```bash
# once the container is running, download a model
curl -X POST http://localhost:11434/download -d '{"model": "llama3.1:8b"}'
```
