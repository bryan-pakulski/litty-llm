#!/bin/bash

if [ ! -d "build/litty-llm-bin" ]; then
    echo "build/litty-llm-bin directory not present, please build & package the project first"
    exit 1
fi

cd build/litty-llm-bin
./litty-llm