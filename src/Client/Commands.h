// Description of commands and the parameters we can use when sending to server
#pragma once
#include <yaml-cpp/node/node.h>
#include "Config/config.h"
#include "Config/structs.h"
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
    - use_cpu                       (Force the system to keep the model in system RAM over GPU)
*/
class loadModelToMemory : public command {
public:
  loadModelToMemory(const ModelConfig &model) {
    m_name = "loadModel";

    m_parameters.push_back(makePair("model_path", model.model_path));
    m_parameters.push_back(makePair("use_cpu", model.use_cpu));
  }
};

} // namespace commands
