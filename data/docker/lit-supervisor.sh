#!/bin/bash

# Restarts the supervisord process which is responsible for keeping the sd model server alive

echo "Reloading supervisord"
unlink /run/supervisor.sock
service supervisor stop

# Run supervisor (or restart if already running)
if pgrep -f "lit-server.py"; then
    echo "Model server already running, killing process"
    kill $(pgrep -f "lit-server.py")
fi

echo "Starting up super visor..."
/usr/bin/supervisord