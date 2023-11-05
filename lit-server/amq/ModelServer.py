import logging
import zmq
import json
import traceback

from MQServer import MQServer

class ModelServer(MQServer):

    def __init__(self):
        MQServer.__init__(self)
        self.debuglogging = True

        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REP)
        self.socket.bind("tcp://*:5557")

        
        self.commandList = {
            "quit": {
                "help": "Shuts down Model Server",
                "arguments": None,
                "function": self.__quit
            },
            "help": {
                "help": "Prints this message",
                "arguments": None,
                "function": self.__help
            },
            "loadModel": {
                "help": "Loads a llama model into memory",
                "arguments": {
                    "model_path": {
                        "help": "Filepath of the model",
                        "required": True,
                        "type": str
                    },
                    "use_cpu": {
                        "help": "Force the system to keep the model in System RAM",
                        "required": False,
                        "type": bool
                    }
                },
                "function": self.__load_model
            },
            "generate": {
                "help": "Generate a response",
                "arguments": {
                    "prompt": {
                        "help": "Prompt",
                        "required": True,
                        "type": str
                    },
                    "max_tokens" : {
                        "help": "The maximum number of tokens to generate, default 128",
                        "required": False,
                        "type": int
                    },
                    "temperature": {
                        "help": "A value controlling the randomness of the sampling process. Higher values result in more random samples, default 0.8",
                        "required": False,
                        "type": float
                    },
                    "top_p": {
                        "help": "The top-p value to use for sampling, deafult 0.95",
                        "required": False,
                        "type": float
                    },
                    "top_k": {
                        "help": "The number of top most probable tokens to consider in the sampling process",
                        "required": False,
                        "type": float
                    }
                },
                "function": self.__generate
            }
        }

        self.model = None   

    # Function commands
    def __quit(self, cmd):
        self.running = False
        self.logging.info("Recieved Quit command")

        return "Quitting..."

    def __help(self, cmd):
        dump = json.dumps(self.commandList, sort_keys=True,
                          indent=4, default=lambda o: f"<<non serializable: {type(o).__qualname__} >>")
        self.logging.info(f"Help dump: {dump}")
        return dump

    def __load_model(self, cmd):
        self.logging.info(f"Loading model: {cmd.arguments['model_path']}")

        # Free the old model if it exists and recover resources
        if (self.model != None):
            self.logging.info(
                f"Freeing old model & recovering resource: {self.model.model_path}")
            self.model.clean()

        self.model = LLAMAModel(**cmd.arguments)
        self.model.load_model()

        return f"Model loaded... {cmd.arguments['model_path']}"

    def __generate(self, cmd):
        result = self.model.generate(**cmd.arguments)
        return f"{result}"

    # Main loop
    def main(self):
        response = ""
        self.logging.info("LLM Server waiting for request...")
        
        while self.running:

            message = self.socket.recv()
            self.logging.info("LLM Server Received request: %s" % message)

            # Break down message, message format is as follows: command:var1=val1,var2=val2 ...
            try:
                cmd = self.parseMessage(message)
                if (self.validateCommand(cmd)):
                    response = self.hookCommand(cmd)
                else:
                    response = "FAILED: Invalid command"
            except Exception as e:
                self.logging.error(traceback.format_exc())
                response = f"FAILED: Exception - {traceback.format_exc()}"

            #  Send reply back to client
            time.sleep(0.5)
            self.socket.send(response.encode())
