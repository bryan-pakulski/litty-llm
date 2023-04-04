# Configurationg

Each model should have its own unique `.json` config file stored in this direcotry, the JSON file describes some of the weights as well as the paths for all the relevant model information, the basic structure is as follows:

```
{
  // Number of layers to load
  "n_layer": 32,
  // Head layer
  "n_head": 32,
  // Embedded
  "n_embd": 4096,
  // Checkpoint paths, the /checkpoints/models folder is treated as the root path
  "checkpoint_paths": [
    "7B/model-00001-of-00002.safetensors",
    "7B/model-00002-of-00002.safetensors"
  ],
  // Tokenizer path, the /checkpoints/models folder is treated as the root path
  "tokenizer_path": "7B/tokenizer.model"
}

```
