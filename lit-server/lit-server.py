from amq import *

import time
import threading
import logging

# Initialise logging
logging.basicConfig(
    filename="/logs/lit-server.log",
    level=logging.INFO,
    format="%(asctime)s - [%(levelname)s] - %(module)s -: %(message)s",
    datefmt='%a %b %d %H:%M:%S %Y'
)

logging.addLevelName(logging.INFO, "info")
logging.addLevelName(logging.WARNING, "warn")
logging.addLevelName(logging.ERROR, "err")
logging.addLevelName(logging.DEBUG, "dbg")

def main():
    # Start model server on seperate thread running in background
    logging.info("Starting Model Server")
    llm_server = amq.ModelServer.ModelServer()
    server_thread = threading.Thread(target =  llm_server.main)
    server_thread.start()

    # Kill main process when heartbeat stops
    # This should only happen when the client is shut down and we don't receive a poll for over 5s
    logging.info("Starting Heartbeat server")
    heartbeat_server = amq.HeartBeat.HeartBeatServer()
    heartbeat_thread = threading.Thread(target = heartbeat_server.main)
    heartbeat_thread.start()
    heartbeat_thread.join()

    llm_server.quit({})
    server_thread.join()


if __name__ == "__main__":
    main()
    
    logging.info("Server shutting down...")
