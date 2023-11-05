#pragma once

#include <fstream>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>

#include <Display/ErrorHandler.h>
#include <Helpers/QLogger.h>
#include "config.h"
#include "structs.h"

namespace MODEL_CONFIG {

static ModelConfig loadModelConfig(std::string hash) {
  ModelConfig config;

  try {
    YAML::Node data = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get())["models"][hash];

    config.name = data["name"].as<std::string>();
    config.hash = data["hash"].as<std::string>();
    config.model_path = data["model_path"].as<std::string>();
    config.use_cpu = data["use_cpu"].as<bool>();

  } catch (const YAML::Exception &err) {
    ErrorHandler::GetInstance().setError("Configuration error", "Failed to read MODELS_CONFIGURATION FILE");
  }

  return config;
}

static void saveModelConfig(const ModelConfig &config) {
  // Build yaml node to attach to model configuration file
  try {
    YAML::Node model_node;
    model_node["name"] = config.name;
    model_node["hash"] = config.hash;
    model_node["model_path"] = config.model_path;
    model_node["use_cpu"] = config.use_cpu;

    // Retrieve root node and dump back to file
    YAML::Node node, _baseNode = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get());
    _baseNode["models"][config.hash] = model_node;
    std::ofstream fout(CONFIG::MODELS_CONFIGURATION_FILE.get());
    fout << _baseNode;
  } catch (const YAML::Exception &err) {
    ErrorHandler::GetInstance().setError("Configuration error",
                                         "Failed to save configuration file" + CONFIG::MODELS_CONFIGURATION_FILE.get());
  }
}

} // namespace MODEL_CONFIG