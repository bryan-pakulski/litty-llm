#pragma once
#include <string>
#include "Client/Commands.h"
#include "Config/config.h"
#include "Helpers/States.h"
#include "ThirdParty/cppzmq/zmq.hpp"

// Singleton class implementation, functions are called on seperate threads, mutex lock on socket access
class Client {
public:
  static Client &GetInstance() {
    static Client s_client;
    return s_client;
  }

  // Prohibit external replication constructs
  Client(Client const &) = delete;
  // Prohibit external assignment operations
  void operator=(Client const &) = delete;

  // Commands list
  void heartbeat(int &state);
  void releaseMemory();
  void loadModelToMemory(commands::loadModelToMemory cmd);

private:
  zmq::context_t m_ctx;
  zmq::socket_t m_socket;
  zmq::socket_t m_heartbeatSocket;
  int m_dockerCommandStatus = Q_COMMAND_EXECUTION_STATE::PENDING;

  std::mutex m_mutex;

  std::string m_addr = "tcp://" + CONFIG::DOCKER_IP_ADDRESS.get() + ":5555";
  std::string m_heartbeat_addr = "tcp://" + CONFIG::DOCKER_IP_ADDRESS.get() + ":5556";
  std::string s_failedResponse = "FAILED";

  std::string sendMessage(const std::string &message);

  Client();
  ~Client();
};