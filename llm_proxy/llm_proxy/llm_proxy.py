'''
llm proxy

a rest proxy for accessing functions from langchain
'''

import os
from uvicorn.main import run
from fastapi import FastAPI


class LLMProxy():
    """LLMProxy class"""

    def __init__(self, host='localhost', port=5000):
        # declare parameters
        self.host = host
        self.port = port
        self.api_url = f"http://{self.host}:{self.port}"

        # env variables
        self.llm_api_key = os.getenv("LLM_API_KEY")

        # create fastapi server
        self.app = FastAPI(
            title="llm proxy",
            version="0.1",
            description="A proxy server for langchain",
        )

        # add routes
        # routes implement the openai api
        # index
        @self.app.get("/")
        def index():
            return {"message": "Welcome to langchain proxy server"}

        # status
        @self.app.get("/status")
        def status():
            return {"status": "ok"}

        # completion
        @self.app.post("/v1/chat/completions")
        def completion(request):
            pass
            # get body
            # body = request.json()

            # # create request to other server
            # res = requests.post(
            #     "https://api.openai.com/v1/chat/completions",
            #     headers={
            #         "Authorization": f"Bearer {self.llm_api_key}"
            #     },
            #     body=body
            # )

            # # return response
            # return res.json()

    def run_fastapi(self):
        """run fastapi server"""
        run(self.app, host=self.host, port=self.port)


def main():
    """main function"""
    llm_proxy = LLMProxy(port=3000)
    llm_proxy.run_fastapi()


if __name__ == '__main__':
    main()
