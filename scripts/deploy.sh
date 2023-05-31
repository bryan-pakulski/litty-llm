#!/bin/bash
if [ ! -d "build" ]; then
    echo "build directory not present, please build the project with cmake first"
    exit 1
fi

cd build/litty-llm-bin
./start_docker.sh