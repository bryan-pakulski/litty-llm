# Starts and runs a model server, this maintains an active model in memory
# Should improve performance without having to reload constantly
from model import LLAMAModel

# Use a messaging queue
import time
import zmq
import json
import logging
import traceback
import threading

context = zmq.Context()
socket = context.socket(zmq.REP)
heartbeatSocket = context.socket(zmq.REP)
socket.bind("tcp://*:5555")
heartbeatSocket.bind("tcp://*:5556")

# Initialise logging
logging.basicConfig(
    filename="/logs/lit-server.log",
    level=logging.INFO,
    format="%(asctime)s - [%(levelname)s][dkr]: %(message)s",
    datefmt='%a %b %d %H:%M:%S %Y'
)

logging.addLevelName(logging.INFO, "info")
logging.addLevelName(logging.WARNING, "warn")
logging.addLevelName(logging.ERROR, "err")
logging.addLevelName(logging.DEBUG, "dbg")


class MQServer():
    def __init__(self):
        self.running = True
        self.debuglogging = False
        self.commandList = {}
        
    # Create a command from a message
    def parseMessage(self, message):
        if self.debuglogging:
            logging.info(f"Recieved message: {message.decode()}")
        return Command(message.decode())

    # Validate a command is correct against the command list
    def validateCommand(self, cmd):
        command_name = cmd.command
        command_args = cmd.arguments

        # Get the argument requirements from the commandList
        if not command_name in self.commandList:
            logging.error(f"Command not found: {command_name}")
            return False

        # Return immediately if arguments aren't required
        command_arg_requirements = self.commandList[command_name]["arguments"]
        if command_arg_requirements == None:
            return True

        # Check if the command has all the required arguments
        for arg_name, arg_details in command_arg_requirements.items():
            # If the argument is required, check if it is present in the command
            if arg_details["required"] and arg_name not in command_args:
                # Return False if the argument is missing
                logging.error(f"Missing required argument: {arg_name}")
                return False

            # Set type
            if arg_name in command_args:
                cmd.arguments[arg_name] = arg_details["type"](
                    cmd.arguments[arg_name])

        # Return True if the command has all the required arguments
        return True

    # Sets up the callback for command
    def hookCommand(self, cmd):
        return self.commandList[cmd.command]["function"](cmd)

class Command():

    def __init__(self, command):
        self.command = None
        self.arguments = {}
        self.parse(command)

    def parse(self, command):
        # Split the message into the command and arguments
        command_and_args = command.split(":")
        self.command = command_and_args[0]

        print(command_and_args)
        if (len(command_and_args) > 1):
            for pair in command_and_args[1:]:
                print(pair)
                arg, val = pair.split("=")
                self.arguments[arg] = val

class HeartBeatServer(MQServer):

    def __init__(self):
        MQServer.__init__(self)
        self.commandList = {
            "ping": {
                "help": "pong",
                "arguments": {},
                "function": self.__pong
            }
        }

    def __pong(self, cmd):
        return "pong"

    # Main loop
    def main(self):
        heartbeatSocket.setsockopt(zmq.RCVTIMEO, 10000)
        response = ""

        logging.info(f"Heartbeat listener started")

        while self.running:
            #  Wait for next request from client
            try: 
                message = heartbeatSocket.recv()
            except zmq.error.Again:
                print("Heartbeat timeout")
                logging.info(f"heartbeat timeout detected, 10000ms without a response")
                self.running = False
                break

            # We received a response
            try:
                cmd = self.parseMessage(message)
                if (self.validateCommand(cmd)):
                    response = self.hookCommand(cmd)
                else:
                    response = "Invalid command"
            except Exception as e:
                logging.error(traceback.format_exc())
                response = f"Exception: {traceback.format_exc()}"

            #  Send reply back to client
            heartbeatSocket.send(response.encode())

        heartbeatSocket.close()
        

class ModelServer(MQServer):

    def __init__(self):
        MQServer.__init__(self)
        self.debuglogging = True
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
                    "context_size": {
                        "help": "Maximum context size",
                        "required": False,
                        "type": int
                    },
                    "keep_in_ram": {
                        "help": "Force the system to keep the model in RAM",
                        "required": False,
                        "type": bool
                    },
                    "lora_base": {
                        "help": "Optional path to base model, useful if using a quantized base model and you want to apply LoRA to an f16 model",
                        "required": False,
                        "type": str
                    },
                    "lora_path": {
                        "help": "Path to a LoRA file to apply to the model",
                        "required": False,
                        "type": str
                    },
                    "seed": {
                        "help": "Random seed. 0 for random",
                        "required": False,
                        "type": int
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
        logging.info("Recieved Quit command")

        return "Quitting..."

    def __help(self, cmd):
        dump = json.dumps(self.commandList, sort_keys=True,
                          indent=4, default=lambda o: f"<<non serializable: {type(o).__qualname__} >>")
        logging.info(f"Help dump: {dump}")
        return dump

    def __load_model(self, cmd):
        logging.info(f"Loading model: {cmd.arguments['checkpoint_path']}")

        # Free the old model if it exists and recover resources
        if (self.model != None):
            logging.info(
                f"Freeing old model & recovering resource: {self.model.checkpoint_path}")
            self.model.clean()

        self.model = LLAMAModel(**cmd.arguments)
        self.model.load_model()

        return f"Model loaded... {cmd.arguments['checkpoint_path']}"

    def __generate(self, cmd):
        result = self.model.generate(**cmd.arguments)
        return f"{result}"

    # Main loop
    def main(self):
        response = ""
        logging.info("LLM Server waiting for request...")
        
        while self.running:

            message = socket.recv()
            logging.info("LLM Server Received request: %s" % message)

            # Break down message, message format is as follows: command:var1=val1,var2=val2 ...
            try:
                cmd = self.parseMessage(message)
                if (self.validateCommand(cmd)):
                    response = self.hookCommand(cmd)
                else:
                    response = "Invalid command"
            except Exception as e:
                logging.error(traceback.format_exc())
                response = f"Exception: {traceback.format_exc()}"

            #  Send reply back to client
            time.sleep(0.5)
            socket.send(response.encode())


def main():
    # Start sd model server on seperate thread running in background
    sd_server = ModelServer()
    print("Starting Model Server")
    server_thread = threading.Thread(target =  sd_server.main)
    server_thread.start()

    print("Starting Heartbeat server")
    # Kill main process when heartbeat stops
    # This should only happen when the client is shut down and we don't receive a poll for over 5s
    heartbeat_server = HeartBeatServer()
    heartbeat_thread = threading.Thread(target = heartbeat_server.main)
    heartbeat_thread.start()
    heartbeat_thread.join()

    sd_server.quit({})
    server_thread.join()


if __name__ == "__main__":
    main()

    logging.info("Server shutting down...")
