import torch

# As mounted inside docker
ROOT_CHECKPOINTS_PATH = "/home/litty-llm/checkpoints/"
CONFIG_PATH = ROOT_CHECKPOINTS_PATH + "configs/"


def get_cuda_device_string():
    if (torch.cuda.is_available()):
        print("Using GPU...")
        return "cuda"
    else:
        print("Using CPU...")
        return "cpu"


@dataclass
class LLaMAConfig:
    block_size: int = 4096
    vocab_size: int = 32000
    n_layer: int = 32
    n_head: int = 32
    n_embd: int = 4096
