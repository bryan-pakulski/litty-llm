#include "Client/DockerCommandsInterface.h"
#include <functional>

#include "Helpers/States.h"
#include <memory>

DockerCommandsInterface::DockerCommandsInterface() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "DockerCommandsInterface::DockerCommandsInterface initialising");
  Client::GetInstance();
}

DockerCommandsInterface::~DockerCommandsInterface() {}

// Starts up Model Server
void DockerCommandsInterface::launchModelServer() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO,
                             "DockerCommandsInterface::launchSDModelServer starting up LLM Model Server...");

  // Launch Server inside docker on startup
#ifdef _WIN32
  std::string commandStr = "data\\scripts\\start_llm_server.bat";
  system(commandStr.c_str());
#else
  system("./data/scripts/start_llm_server.sh");
#endif
}

// Release SD Model Server data from gpu memory
void DockerCommandsInterface::releaseModelServer() {
  QLogger::GetInstance().Log(LOGLEVEL::INFO,
                             "SDCommandsInterface::restartSDModelServer shutting down SD Model Server...");
  m_commandState = Q_EXECUTION_STATE::PENDING;
  m_Thread = std::thread(std::bind(&Client::releaseMemory, &Client::GetInstance(), std::ref(m_commandState)));
  m_Thread.detach();
}
// Connect a new model to SD Server
void DockerCommandsInterface::attachModelToServer(std::string model_path) {
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "SDCommandsInterface::attachModelToServer loading model to memory...");
  m_commandState = Q_EXECUTION_STATE::PENDING;

  commands::loadModelToMemory cmd = commands::loadModelToMemory{model_path, 512, true, "", "", 0};
  m_Thread = std::thread(std::bind(&Client::loadModelToMemory, &Client::GetInstance(), cmd, std::ref(m_commandState)));
  m_Thread.detach();
}