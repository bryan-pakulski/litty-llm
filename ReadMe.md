This project aims to encapsulate LLAMA into a docker container with a gui client / server message queue for interaction. It also includes functionality for crawling and indexing files for vector search.

# Requirements:
- Docker / Docker Compose

# Usage

- Run the `./start_docker.sh` script to initialise the llm server
- Once docker is deployed the `entrypoint.sh` script should ensure that the `lit-server` is installed and ready to accept commands on localhost:5555 

    (Note the server will auto shutdown if it doesn't receive heartbeat commands over port 5556, it expects a "ping" command at least every 10s to ensure that a client is connected. If no connection is present the server will stop and release memory, this is only a concern if you are rolling your own client to connect to the back end)
- Launch the litty-llm gui to interact with the LLM server, the top docker status indicates whether a connection has been established.

# Features:

- Load LLM into memory to save on inference time
- Support ZMQ endpoint to interact with LLM server

# Building

There is a github pipeline available for both linux & windows that runs on the master branch.
Release artifacts are saved for each tagged version and published.

If you'd like to build locally you can do so using the cmake build system:

## Linux:

Install supporting libraries:

- `libzmq` / `libzmq-dev` / `libzmq3-dev` (dependant on distribution)
- `yaml-cpp`

### Building:

- Compiler: GCC

- Run the following build scripts:
  - `build.sh -r` compile gui and package docker container, `-r` flag is optional to enable release mode
  - `deploy.sh` Deploy docker changes and synchronise with `compose-up`
  - `run.sh` Run gui
- Build is stored in `build/litty-llm-bin` (statically linked)

## Windows:

- Compiler: VSCC Visual Studio 17 2022

The `build.bat` script assumes the you have vcpkg installed in the root `C:\` drive<br>
Libraries can be installed using vcpkg in the same manner as they are set up in the pipeline:

```
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg install zeromq --triplet x64-windows
```

### Building

- `./scripts/build.bat -r` to build & package application, stored in `build/litty-llm-bin`, `-r` is optional to enable release mode
- `./scripts/deploy.bat` to deploy / update existing docker container
- `./scripts/run.bat` to start application