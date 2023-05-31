import sys
import time

from langchain.llms import LlamaCpp
import os
import argparse

import logging

## Store loaded model in memory to re-query when required
class LLAMAModel():

    def __init__(self, model_path: str = None, context_size: int = 512, keep_in_ram: bool = False, lora_base: str = None, lora_path: str = None, seed: int = 0):
        
        """Loads a pre-trained LLaMA model and tokenizer into memory.

        Args:
            model_path: The model (.pth) path to load.
            tokenizer_path: The tokenizer (.model) path to load.
            model_size: Model size parameter to load
            precision: whether to use "full" or "half" step precision for the checkpoint weights
            quantize: Whether to quantize the model using the `LLM.int8()` method
        """
            
        # Initialise logging
        logging.basicConfig(
            filename="/home/litty-llm/logs/lit-server.log",
            level=logging.INFO,
            format="[SERVER] %(asctime)s - %(levelname)s - %(message)s"
        )

        logging.addLevelName(logging.INFO, "INFO")
        logging.addLevelName(logging.WARNING, "WARN")
        logging.addLevelName(logging.ERROR, "ERR")

        # Model
        self.model = None

        # Generation parameters
        self.model_path = model_path
        self.keep_in_ram = keep_in_ram
        self.context_size = context_size
        self.seed = seed
        self.lora_base = lora_base
        self.lora_path = lora_path


    def load_model(self):
        self.model = LlamaCpp(model_path=self.model_path, n_ctx=self.context_size, use_mlock=self.keep_in_ram, lora_base=self.lora_base, lora_path=self.lora_path, seed=self.seed)

    def clean(self):
        if (self.model is not None):
            del self.model

    def generate(self, prompt: str = "", max_tokens: int = 128, temperature: float = 0.8, top_p: float = 0.95, top_k: float = 40):
        return self.model(prompt, max_tokens=max_tokens, temperature=temperature, top_p=top_p, top_k=top_k)
    
