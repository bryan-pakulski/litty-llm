# Initialise docker container
if [ ! -d "data/docker" ]; then
    echo "docker data directory not present, please run this script from the same directory as litty-llm"
    exit 1
fi

# Ensure log files have correct permissions, create as current user if they don't exist
touch data/logs/lit-server.log
touch data/logs/docker-server.err
touch data/logs/docker-server.out

pushd data/docker

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