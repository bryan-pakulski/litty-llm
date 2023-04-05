import torch
import json
import os

from dataclasses import dataclass

# As mounted inside docker
ROOT_CHECKPOINTS_PATH = "/checkpoints/"
CONFIG_PATH = ROOT_CHECKPOINTS_PATH + "configs/"
MODEL_PATH = ROOT_CHECKPOINTS_PATH + "models/"


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

    checkpoint_paths = []
    tokenizer_path = ""

    def __init__(self, config_file):
        with open(f"{CONFIG_PATH}/{config_file}.json") as f:
            config_data = json.load(f)

        self.n_layer = config_data.get('n_layer', self.n_layer)
        self.n_head = config_data.get('n_head', self.n_head)
        self.n_embd = config_data.get('n_embd', self.n_embd)
        self.checkpoint_paths = config_data.get(
            'checkpoint_paths', self.checkpoint_paths)
        self.tokenizer_path = config_data.get(
            'tokenizer_path', self.tokenizer_path)

        # TODO: asset config file
