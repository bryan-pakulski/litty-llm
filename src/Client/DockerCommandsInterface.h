#pragma once

#include "Config/config.h"
#include "Config/structs.h"
#include "Helpers/QLogger.h"

#include "Client.h"
#include "Helpers/States.h"

#include <memory>
#include <vector>

// Singleton class implementation
// Runs on seperate thread for non-blocking operations
class DockerCommandsInterface {
private:
  std::thread m_Thread;

  DockerCommandsInterface();
  ~DockerCommandsInterface();

public:
  int m_commandState = Q_EXECUTION_STATE::PENDING;

  static DockerCommandsInterface &GetInstance() {
    static DockerCommandsInterface s_ci;
    return s_ci;
  }

  // Prevent replication
  DockerCommandsInterface(DockerCommandsInterface const &) = delete;
  void operator=(DockerCommandsInterface const &) = delete;

  // Model Server Specific Commands
  void launchModelServer();
  void releaseModelServer();
  void attachModelToServer(std::string model_path);
};