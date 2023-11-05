import logging

class MQServer():
    def __init__(self):
        self.logging = logging.getLogger(__name__)
        self.running = True
        self.debuglogging = False
        self.commandList = {}
        
    # Create a command from a message
    def parseMessage(self, message):
        if self.debuglogging:
            self.logging.info(f"Recieved message: {message.decode()}")
        return Command(message.decode())

    # Validate a command is correct against the command list
    def validateCommand(self, cmd):
        command_name = cmd.command
        command_args = cmd.arguments

        # Get the argument requirements from the commandList
        if not command_name in self.commandList:
            self.logging.error(f"Command not found: {command_name}")
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
                self.logging.error(f"Missing required argument: {arg_name}")
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
