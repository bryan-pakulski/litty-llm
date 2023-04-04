This project is build off of the lit-llama project found here:
https://github.com/Lightning-AI/lit-llama

The main functionality improvment is encapsulating the LLM into a docker container with a client / server message queue for interaction.

Support is also included for the following:

- Safe tensors
- Original LLAMA model support
- LIT-LLAMA support
- 8 bit Quantization
- 4 bit Quantization (TODO)
- Model Layering (Shared resources between GPU / CPU) (TODO)

## Checkpoint configuration:

In order to use this model you will need the model files stored in the `checkpoints/models` directory. You will also need to create a configuration for each model that
can be stored in the `checkpoints/configs` directory, see the `Readme.md` file for instructions on how to set that up

## Requirements:

- Docker
- Docker Compose
- Python3 (and supporting libraries installed)
  - zmq

## Installation & Usage

- Run the `./deploy_litty` script in the main project directory
- Once the docker container is deployed the `entrypoint.sh` script should run automatically and ensure that the `lit-server` is running and ready to accept commands on tcp://localhost:5555
- Use the example `lit-client.py` script to interact with the server to confirm that everything is working as intended i.e. `lit-client.py help`
- For any unexpected behaviour check the `/log` directory

## Features currently supported:

- Load LLM into memory to save on inference time
- Support different compute devices i.e. cpu, gpu etc..
- Support 8bit quanitzation for consumer VRAM hardware
- Support ZMQ endpoint to interact with LLM server

## Example commands:

### Loading Model to memory:

Note: that paths for the model default to the "checkpoint" directory that is linked to the docker container from the working directory, that is treated as the root path for the model files

```
python lit-client.py loadModel:accelerator=auto:config="7B-HF4bit":precision=full:quantize=True
```

### Querying loaded model:

```
python lit-client.py generate:prompt="What is the meaning of life?"
```

### View help menu:

```
python lit-client.py help
```

## TODO

- Add additional optimisation on the tensors to reduce storage space
- Offload some storage onto the CPU
- Look into CUDA device optimisation:\
   `` You are using a CUDA device ('NVIDIA GeForce RTX 3060') that has Tensor Cores. To properly utilize them, you should set `torch.set_float32_matmul_precision('medium' | 'high')` which will trade-off precision for performance. For more details, read https://pytorch.org/docs/stable/generated/torch.set_float32_matmul_precision.html#torch.set_float32_matmul_precision ``
