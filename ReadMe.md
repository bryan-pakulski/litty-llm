This project is build off of the lit-llama project found here:
https://github.com/Lightning-AI/lit-llama

The main functionality improvment is encapsulating the LLAMA into a docker container with a client / server message queue for interaction.

## Checkpoint configuration:

In order to use this model you will need the checkpoint files converted and stored in the `checkpoints` directory. The original conversion script is included from `lit-llama`, full
details can be found on their repo: https://github.com/Lightning-AI/lit-llama

## Installation

- Run the `./deploy_litty.sh` script in the main project directory
- Once docker is deployed the `entrypoint.sh` script should ensure that the `lit-server` is running and ready to accept commands
- The docker container should now have the server running and waiting for commands on localhost:5555
- Use the example `lit-client.py` script to interact with the server to confirm that everything is working as intended i.e. `lit-client.py help`

## Features currently supported:

- Load LLM into memory to save on inference time
- Support different compute devices i.e. cpu, gpu etc..
- Support 8bit quanitzation for consumer VRAM hardware
- Support ZMQ endpoint to interact with LLM server

## Example commands:

### Loading Model to memory:

Note: that paths for the model default to the "checkpoint" directory that is linked to the docker container from the working directory, that is treated as the root path for the model files

```
python lit-client.py loadModel:accelerator=auto:checkpoint_path=lit-llama/7B/state_dict.pth:model_size=7B:precision=full:quantize=True:tokenizer_path=lit-llama/tokenizer.model
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
    ```You are using a CUDA device ('NVIDIA GeForce RTX 3060') that has Tensor Cores. To properly utilize them, you should set `torch.set_float32_matmul_precision('medium' | 'high')` which will trade-off precision for performance. For more details, read https://pytorch.org/docs/stable/generated/torch.set_float32_matmul_precision.html#torch.set_float32_matmul_precision```