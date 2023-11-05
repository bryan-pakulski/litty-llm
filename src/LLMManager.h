#pragma once

#include "Config/structs.h"
#include "Helpers/States.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <memory>
#include <thread>
#include <vector>

/*
  This class is responsible for providing a singleton stateful interface to the model server running on docker
*/

class LLMManager {
public:
  std::vector<std::string> m_latestFiles;

public:
  static LLMManager &GetInstance() {
    static LLMManager s_llmManager;
    return s_llmManager;
  }

  // Prohibit external replication constructs
  LLMManager(LLMManager const &) = delete;
  // Prohibit external assignment operations
  void operator=(LLMManager const &) = delete;

  /*
  DOCKER SERVER INTERACTION
  */

  void launchLLMModelServer();
  void releaseLLMModel();
  void attachModel(ModelConfig model);
  const ModelConfig &getLoadedModel() { return m_model; }

  int getModelState() { return m_modelState; }
  void setModelState(int state) { m_modelState = state; }

  /*
    CALLBACKS
  */
  static void GLFWErrorCallBack(int, const char *err_str);
  static void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                         const GLchar *message, const void *userParam);

private:
  int m_modelState = Q_MODEL_STATUS::NONE_LOADED;
  int m_dockerState = Q_COMMAND_EXECUTION_STATE::NONE;

  ModelConfig m_model;
  std::thread m_Thread;

private:
  explicit LLMManager();
  ~LLMManager();
};