#!/bin/bash

if [ ! -d "build" ]; then
    echo "build directory not present, please build the project with cmake first"
    exit 1
fi

mkdir -p build/litty-llm-bin

if [ -f build/litty-llm ]; then
  mv build/litty-llm build/litty-llm-bin/
fi

cp -ruv data build/litty-llm-bin/ 
cp -ruv lit-server build/litty-llm-bin/data/docker/
cp -uv scripts/docker/start_docker.sh build/litty-llm-bin/