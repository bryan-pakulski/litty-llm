// Description of commands and the parameters we can use when sending to server
#pragma once
#include <yaml-cpp/node/node.h>
#include "Config/config.h"
#include <vector>
#include <string>
#include <sstream>

// TODO: get additional configurations for certain commands

// Base command class
class command {
public:
  std::string m_name;
  std::vector<std::pair<std::string, std::string>> m_parameters;

  // Return a formatted and delimited command string i.e. "command:k1=v1:k2=v2"
  std::string getCommandString() {
    std::stringstream cmdStream;

    cmdStream << m_name;

    for (auto &kvp : m_parameters) {
      cmdStream << ":" << kvp.first << "=" << kvp.second;
    }
    return cmdStream.str();
  }

  // Make a key value pair
  std::pair<std::string, std::string> makePair(std::string a, std::string b) {
    return std::pair<std::string, std::string>{a, b};
  }
  std::pair<std::string, std::string> makePair(std::string a, double b) {
    return std::pair<std::string, std::string>{a, std::to_string(b)};
  }
  std::pair<std::string, std::string> makePair(std::string a, int b) {
    return std::pair<std::string, std::string>{a, std::to_string(b)};
  }
};

namespace commands {

class heartbeat : public command {
public:
  heartbeat() { m_name = "ping"; }
};

class restartServer : public command {
public:
  restartServer() { m_name = "restart"; }
};

/*
Call command to load stable diffusion model to memory

Parameters:
    - model_path                    (Path to model file inside docker image)
    - context_size                  (Maximum context size)
    - keep_in_ram                   (Force the system to keep the model in RAM)
    - lora_base                     (Optional path to base model)
    - lora_path                     (Path to a LoRA file to apply to the model)
    - seed                          (Random seed. 0 for random)
*/
class loadModelToMemory : public command {
public:
  loadModelToMemory(std::string &model_path, const int &context_size = 512, bool keep_in_ram = false,
                    const std::string &lora_base = "", const std::string &lora_path = "", const int &seed = 0) {
    m_name = "loadModel";

    m_parameters.push_back(makePair("model_path", model_path));
    m_parameters.push_back(makePair("context_size", context_size));
    m_parameters.push_back(makePair("keep_in_ram", keep_in_ram));
    if (lora_base != "") {
      m_parameters.push_back(makePair("lora_base", lora_base));
    }
    if (lora_path != "") {
      m_parameters.push_back(makePair("lora_path", lora_path));
    }
    m_parameters.push_back(makePair("seed", seed));
  }
};

} // namespace commands
