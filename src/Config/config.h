#pragma once

#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>
#include "types.h"

namespace CONFIG {

static YAML::Node configFile = YAML::LoadFile("data/config.yaml");

// Load config with default value if item doesn't exist
template <class T> static T loadConfig(const std::string &configPath, const T &defaultValue) {
  try {
    return configFile[configPath].as<T>();
  } catch (const YAML::Exception &) {
    return defaultValue;
  }
}

// Prompt user for config, save to file
template <class T> static void setConfig(T &variable, const std::string &configPath) {
  // Call error logging function
}

// Generation configuration
static CInt PROMPT_LENGTH_LIMIT(loadConfig<int>("PROMPT_LENGTH_LIMIT", 200));
static CString MODELS_DIRECTORY(loadConfig<std::string>("MODELS_DIRECTORY", "/models/"));

// ImGui configuration
static CString PROGRAM_NAME("litty-llm");
static CFloat HIGH_DPI_SCALE_FACTOR(loadConfig<float>("HIGH_DPI_SCALE_FACTOR", 1.0f));
static CInt WINDOW_WIDTH(loadConfig("WINDOW_WIDTH", 1280));
static CInt WINDOW_HEIGHT(loadConfig("WINDOW_HEIGHT", 720));

static CFloat IMGUI_LOG_WINDOW_HEIGHT(loadConfig("IMGUI_LOG_WINDOW_HEIGHT", 900.0f));
static CFloat IMGUI_LOG_WINDOW_WIDTH(loadConfig("IMGUI_LOG_WINDOW_WIDTH", 820.0f));

// Program configuration
static CInt DEFAULT_BUFFER_LENGTH(loadConfig("DEFAULT_BUFFER_LENGTH", 200));
static CString DOCKER_IP_ADDRESS(loadConfig<std::string>("DOCKER_IP_ADDRESS", ""));

static CString MODEL_CONFIGURATIONS_DIRECTORY(loadConfig<std::string>("MODEL_CONFIGURATIONS_DIRECTORY",
                                                                      "data/models/configs"));
static CString MODELS_CONFIGURATION_FILE(loadConfig<std::string>("MODELS_CONFIGURATION_FILE",
                                                                 "data/config/models.yaml"));

// Logging
static CInt ENABLE_GL_DEBUG(loadConfig<int>("ENABLE_OPENGL_DEBUG_OUTPUT", 0));
static CInt ENABLE_TRACE_LOGGING(loadConfig<int>("ENABLE_TRACE_LOGGING", 0));
static CInt DEBUG_LEVEL(loadConfig<int>("DEBUG_LEVEL", 5));
static CString LOG_FILE(loadConfig<std::string>("LOG_FILE", "data/logs/llm-ui.log"));

} // namespace CONFIG