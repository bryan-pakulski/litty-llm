#pragma once

#include <string>

struct ModelConfig {
  std::string name = "";
  std::string model_path = "";
  std::string hash = "";

  bool use_cpu = false;
};
