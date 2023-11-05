#include "LLMManager.h"

#include "Client/Commands.h"
#include "Client/Client.h"
#include "Config/config.h"
#include "Config/structs.h"
#include "Helpers/QLogger.h"
#include "Helpers/States.h"
#include <functional>

LLMManager::LLMManager() {
  QLogger::GetInstance().Log(LOGLEVEL::TRACE, "LLMManager::LLMManager");

  // Intialise zmq server within docker to receive commands from client
  launchLLMModelServer();

  // TODO: make a command queue
}

LLMManager::~LLMManager() {}

void LLMManager::launchLLMModelServer() {
  QLogger::GetInstance().Log(LOGLEVEL::TRACE, "LLMManager::launchLLMModelServer");

  // Launch SD Server inside docker on startup
#ifdef _WIN32
  std::string commandStr = "data\\scripts\\start_sd_server.bat";
  system(commandStr.c_str());
#else
  system("./data/scripts/start_llm_server.sh");
#endif
}

// Send Command to attach a model on the docker server
void LLMManager::attachModel(ModelConfig modelConfig) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "LLMManager::attachModel Attaching model: ", modelConfig.name,
                             " to Stable Diffusion Docker Server");
  m_model = modelConfig;
  setModelState(Q_MODEL_STATUS::LOADING);

  commands::loadModelToMemory cmd = commands::loadModelToMemory{modelConfig};

  m_Thread = std::thread(std::bind(&Client::loadModelToMemory, &Client::GetInstance(), cmd));
  m_Thread.detach();
}

void LLMManager::releaseLLMModel() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "LLMManager::releaseSDModel releasing model from server...");

  m_Thread = std::thread(std::bind(&Client::releaseMemory, &Client::GetInstance()));
  m_Thread.detach();
}

// Callback to log GL errors
void LLMManager::GLFWErrorCallBack(int, const char *err_str) { QLogger::GetInstance().Log(LOGLEVEL::ERR, err_str); }

void GLAPIENTRY LLMManager::MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                            const GLchar *message, const void *userParam) {
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}