# Starts and runs a model server, this maintains an active model in memory
# Should improve performance without having to reload constantly
from model import LLAMAModel

# Use a messaging queue
import time
import zmq
import json
import logging
import traceback

context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:5555")


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


class LitModelServer():

    def __init__(self):
        self.running = True
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
                    "accelerator": {
                        "help": "Hardware accelerator to use i.e [cpu, cuda, mps, gpu, tpu, auto]",
                        "required": False,
                        "type": str
                    },
                    "config": {
                        "help": "json config file path",
                        "required": True,
                        "type": str
                    },
                    "quantize": {
                        "help": "Whether to quantize the model & save VRAM at the cost of accuracy: [Full, 8bit, 4bit, 3bit]",
                        "required": False,
                        "type": str
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
                    "num_samples": {
                        "help": "Number of responses to generate for the prompt, default 1",
                        "required": False,
                        "type": int
                    },
                    "max_new_tokens": {
                        "help": "Number of generation steps to take, default 50",
                        "required": False,
                        "type": int
                    },
                    "top_k": {
                        "help": "The number of top most probable tokens to consider in the sampling process.",
                        "required": False,
                        "type": int
                    },
                    "temperature": {
                        "help": "A value controlling the randomness of the sampling process. Higher values result in more random samples.",
                        "required": False,
                        "type": int
                    },
                    "seed": {
                        "help": "Random seed for reproducibility, default to 1234",
                        "required": False,
                        "type": int
                    }
                },
                "function": self.__generate
            }
        }

        self.model = None

        # Initialise logging
        logging.basicConfig(
            filename="/logs/lit-server.log",
            level=logging.INFO,
            format="[SERVER] %(asctime)s - %(levelname)s - %(message)s"
        )

        logging.addLevelName(logging.INFO, "INFO")
        logging.addLevelName(logging.WARNING, "WARN")
        logging.addLevelName(logging.ERROR, "ERR")

        logging.info("Starting up llama server")

    # Create a command from a message
    def parseMessage(self, message):
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
            # TODO: boolean typesetting may cause issues in casting
            if arg_name in command_args:
                cmd.arguments[arg_name] = arg_details["type"](
                    cmd.arguments[arg_name])

        # Return True if the command has all the required arguments
        return True

    # Sets up the callback for command
    def hookCommand(self, cmd):
        return self.commandList[cmd.command]["function"](cmd)

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
        logging.info(
            f"Loading model from config: checkpoints/configs/{cmd.arguments['config']}.json")

        # Free the old model if it exists and recover resources
        if (self.model != None):
            logging.info(
                f"Freeing old model & recovering resources")
            self.model.clean()

        self.model = LLAMAModel(**cmd.arguments)
        self.model.load_model()

        return f"Model loaded..."

    def __generate(self, cmd):
        result = self.model.generate(**cmd.arguments)
        return f"{result}"

    # Main loop
    def main(self):
        response = ""

        while self.running:
            #  Wait for next request from client
            logging.info("LLM Server waiting for request...")

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
    lit_server = LitModelServer()
    lit_server.main()


if __name__ == "__main__":
    main()

    logging.info("Server shutting down...")
