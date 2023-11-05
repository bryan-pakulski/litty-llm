import sys
import time

import os
import argparse

import logging

## Store loaded model in memory to re-query when required
class LLAMAModel():

    def __init__(self, model_path: str = None, use_cpu: bool = False):
        self.logging = logging.getLogger(__name__)

        # Model
        self.model = None

        # Generation parameters
        self.model_path = model_path
        self.use_cpu = use_cpu


    def load_model(self):
        pass

    def clean(self):
        if (self.model is not None):
            del self.model

    def generate(self, prompt: str = "", max_tokens: int = 128, temperature: float = 0.8, top_p: float = 0.95, top_k: float = 40):
        return self.model(prompt, max_tokens=max_tokens, temperature=temperature, top_p=top_p, top_k=top_k)
    
