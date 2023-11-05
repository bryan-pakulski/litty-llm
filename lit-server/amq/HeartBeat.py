from MQServer import MQServer

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

        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REP)
        self.socket.bind("tcp://*:5558")

    def __pong(self, cmd):
        return "pong"

    # Main loop
    def main(self):
        self.socket.setsockopt(zmq.RCVTIMEO, 10000)
        response = ""

        self.logging.info(f"Heartbeat listener started")

        while self.running:
            #  Wait for next request from client
            try: 
                message = self.socket.recv()
            except zmq.error.Again:
                print("Heartbeat timeout")
                self.logging.info(f"heartbeat timeout detected, 10000ms without a response")
                self.running = False
                break

            # We received a response
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
            self.socket.send(response.encode())

        self.socket.close()
        
