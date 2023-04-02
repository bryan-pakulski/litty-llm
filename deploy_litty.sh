#!/bin/bash

pushd docker

# Initialise docker container
docker build -t litty-llm .
docker compose --file docker-compose-nvidia.yml up -d

docker exec litty-llm /home/entrypoint.sh

popd