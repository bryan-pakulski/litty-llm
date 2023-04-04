import os
import sys
import time
from pathlib import Path
from typing import Optional

import lightning as L
import safetensors.torch
import torch

import logging

from torch.utils.data import DataLoader
from lit_llama import LLaMA, Tokenizer, as_8_bit_quantized

# Helper functions
import common

# Store loaded model in memory to re-query when required


class LLAMAModel():

    def __init__(self, accelerator: str = "auto", config: str = "7B", precision: str = "full", quantize: bool = False):
        """Loads a pre-trained LLaMA model and tokenizer into memory.

        Args:
            accelerator: The hardware to run on. Possible choices are:
                ``"cpu"``, ``"cuda"``, ``"mps"``, ``"gpu"``, ``"tpu"``, ``"auto"``.
            config: Model configuration to use [see configs folder]
            precision: whether to use "full" or "half" step precision for the checkpoint weights
            quantize: Whether to quantize the model using the `LLM.int8()` method
        """

        # Initialise logging
        logging.basicConfig(
            filename="/logs/lit-server.log",
            level=logging.INFO,
            format="[SERVER] %(asctime)s - %(levelname)s - %(message)s"
        )

        logging.addLevelName(logging.INFO, "INFO")
        logging.addLevelName(logging.WARNING, "WARN")
        logging.addLevelName(logging.ERROR, "ERR")

        # Generation objects
        self.model = None
        self.checkpoint = None
        self.tokenizer = None
        self.fabric = None

        # Generation parameters
        self.accelerator = accelerator
        self.config = config
        self.precision = precision
        self.quantize = quantize

    def load_model(self):
        self.fabric = L.Fabric(accelerator=self.accelerator, devices=1)

        # TODO: 4bit

        # TODO: safe tensors
        safe_tensors = []

        with as_8_bit_quantized(self.fabric.device, enabled=self.quantize):
            t0 = time.time()
            self.model = LLaMA.from_config(self.config)

            # Create a DataLoader to iterate over all our datasets
            dataset = torch.utils.data.ConcatDataset(
                self.model.checkpoint_paths)
            data_loader = DataLoader(dataset, batch_size=1, shuffle=False)

            # Load model fragment
            # TODO: safetensors
            for data in data_loader:
                print(data)

                torch.load(data)
                self.model.load_state_dict(data)

            # TODO see if this works
            if self.precision != "full":
                self.model.half()

            logging.debug(
                f"Time to load model: {time.time() - t0:.02f} seconds.")

        self.model.eval()

        self.model = self.fabric.setup_module(self.model)
        self.tokenizer = Tokenizer(self.model.tokenizer_path)

    def clean(self):
        if torch.cuda.is_available():
            with torch.cuda.device(common.get_cuda_device_string()):
                torch.cuda.empty_cache()
                torch.cuda.ipc_collect()

        if (self.model is not None):
            del self.model

    @torch.no_grad()
    def __generate(self, idx: torch.Tensor, max_new_tokens: int, max_seq_length: int, temperature: float, top_k: int) -> torch.Tensor:
        """Takes a conditioning sequence (prompt) as input and continues to generate as many tokens as requested.
            The implementation of this function is modified from A. Karpathy's nanoGPT.

        Args:
            idx: Tensor of shape (B, T) with indices of the prompt sequence.
            max_new_tokens: The number of new tokens to generate.
            max_seq_length: The maximum sequence length allowed.
            temperature: Scales the predicted logits by 1 / temperature
            top_k: If specified, only sample among the tokens with the k highest probabilities
        """
        # create an empty tensor of the expected final shape and fill in the current tokens
        B, T = idx.shape
        T_new = T + max_new_tokens
        empty = torch.empty(B, T_new, dtype=idx.dtype, device=idx.device)
        empty[:, :T] = idx
        idx = empty

        # generate max_new_tokens tokens
        for t in range(T, T_new):
            # ignore the not-filled-yet tokens
            idx_cond = idx[:, :t]
            # if the sequence context is growing too long we must crop it at max_seq_length
            idx_cond = idx_cond if T <= max_seq_length else idx_cond[:, -max_seq_length:]

            # forward
            logits = self.model(idx_cond)
            logits = logits[:, -1] / temperature

            # optionally crop the logits to only the top k options
            if top_k is not None:
                v, _ = torch.topk(logits, min(top_k, logits.size(-1)))
                logits[logits < v[:, [-1]]] = -float("Inf")

            probs = torch.nn.functional.softmax(logits, dim=-1)
            idx_next = torch.multinomial(probs, num_samples=1)

            # concatenate the new column
            idx[:, t:] = idx_next

        return idx

    def generate(self, prompt: str = "Hello World!", num_samples: int = 1, max_new_tokens: int = 50, top_k: int = 200, temperature: float = 0.8, seed: int = 1234):
        """Generates text samples based on a pre-trained LLaMA model and tokenizer.
        Args:
            prompt: The prompt string to use for generating the samples.
            num_samples: The number of text samples to generate.
            max_new_tokens: The number of generation steps to take.
            top_k: The number of top most probable tokens to consider in the sampling process.
            temperature: A value controlling the randomness of the sampling process. Higher values result in more random
                samples.
            seed: Random seed for reproducability
        """
        if self.model is None:
            logging.error('Model not loaded..')
            return None

        encoded_prompt = self.tokenizer.encode(
            prompt, bos=True, eos=False, device=self.fabric.device)
        encoded_prompt = encoded_prompt[None, :]  # add batch dimension

        L.seed_everything(seed)
        t0 = time.perf_counter()

        responses = []

        for _ in range(num_samples):
            y = self.__generate(
                encoded_prompt,
                max_new_tokens,
                # type: ignore[union-attr,arg-type]
                self.model.config.block_size,
                temperature=temperature,
                top_k=top_k,
            )[0]  # unpack batch dimension

            responses.append(self.tokenizer.decode(y))

        logging.info(f"Generated Responses: {responses}")

        t = time.perf_counter() - t0
        logging.debug(
            f"\n\nTime for inference: {t:.02f} sec total, {num_samples * max_new_tokens / t:.02f} tokens/sec")
        logging.debug(
            f"Memory used: {torch.cuda.max_memory_reserved() / 1e9:.02f} GB")

        return responses
