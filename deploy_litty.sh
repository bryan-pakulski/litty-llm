#!/bin/bash

pushd docker

# Initialise docker container
docker build -t litty-llm .


# Some distro requires that the absolute path is given when invoking lspci
# e.g. /sbin/lspci if the user is not root.
gpu=$(lspci | grep -i '.* vga .* nvidia .*')

shopt -s nocasematch

if [[ $gpu == *' nvidia '* ]]; then
  printf 'Nvidia GPU is present:  %s\n' "$gpu"
  docker compose --file docker-compose-nvidia.yml up -d
else
  printf 'Nvidia GPU is not present: %s\n' "$gpu"
  docker compose --file docker-compose-cpu.yml up -d
fi

docker exec litty-llm /home/entrypoint.sh

popd