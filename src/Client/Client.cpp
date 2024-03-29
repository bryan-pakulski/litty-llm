#include "Client/Client.h"
#include "Client/Commands.h"
#include "Display/ErrorHandler.h"
#include "Helpers/States.h"
#include "Helpers/QLogger.h"
#include "LLMManager.h"
#include <sstream>

Client::Client() {

  m_ctx = zmq::context_t{1};

  // Initialise general communication socket
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Client::Client Connecting main zmq interface", m_addr);
  m_socket = zmq::socket_t(m_ctx, zmq::socket_type::req);
  m_socket.connect(m_addr);

  // Pending messages shall be discarded immediately when the socket is closed with zmq_close()
  // see: http: //api.zeromq.org/2-1%3azmq-setsockopt#toc15
  m_socket.set(zmq::sockopt::linger, 0);

  // Initialise heartbeat socket for keepalive status
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Client::Client Connecting heartbeat zmq interface", m_heartbeat_addr);
  m_heartbeatSocket = zmq::socket_t(m_ctx, zmq::socket_type::req);
  m_heartbeatSocket.connect(m_heartbeat_addr);
  m_heartbeatSocket.set(zmq::sockopt::rcvtimeo, 5000);
  m_heartbeatSocket.set(zmq::sockopt::linger, 0);
}

Client::~Client() {}

// Heartbeat ping/pong to determine server status
void Client::heartbeat(int &state) {
  commands::heartbeat cmd = commands::heartbeat();
  zmq::message_t msg = zmq::message_t(cmd.getCommandString());
  zmq::message_t pong;

  try {
    m_heartbeatSocket.send(msg, zmq::send_flags::none);

    zmq::recv_result_t r = m_heartbeatSocket.recv(pong, zmq::recv_flags::none);
    if (r.has_value() && (EAGAIN == r.value())) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "Client::heartbeat Invalid ping response!");
      state = HEARTBEAT_STATE::DEAD;
    } else {
      QLogger::GetInstance().Log(LOGLEVEL::DBG1, "Client::heartbeat ", pong.to_string());
      state = HEARTBEAT_STATE::ALIVE;
    }

  } catch (const zmq::error_t &err) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, err.what());
    state = HEARTBEAT_STATE::DEAD;
    return;
  }
}

// Reloads sd server in docker (release any models in memory)
void Client::releaseMemory() {
  commands::restartServer cmd = commands::restartServer();
  std::string msg = sendMessage(cmd.getCommandString());
  if (m_dockerCommandStatus == Q_COMMAND_EXECUTION_STATE::SUCCESS) {
    LLMManager::GetInstance().setModelState(Q_MODEL_STATUS::NONE_LOADED);
  } else {
    LLMManager::GetInstance().setModelState(Q_MODEL_STATUS::FAILED);
  }
}

// Load a stable diffusion model into memory in preperation for running inference commands
void Client::loadModelToMemory(commands::loadModelToMemory cmd) {
  std::string msg = sendMessage(cmd.getCommandString());

  if (m_dockerCommandStatus == Q_COMMAND_EXECUTION_STATE::SUCCESS) {
    LLMManager::GetInstance().setModelState(Q_MODEL_STATUS::NONE_LOADED);
  } else {
    LLMManager::GetInstance().setModelState(Q_MODEL_STATUS::FAILED);
  }
}

// Send message to ZMQ server listening on m_socket
std::string Client::sendMessage(const std::string &message) {
  std::lock_guard<std::mutex> guard(m_mutex);

  m_dockerCommandStatus = Q_COMMAND_EXECUTION_STATE::PENDING;

  m_socket.send(zmq::buffer(message));

  zmq::message_t recv;
  zmq::recv_result_t r = m_socket.recv(recv, zmq::recv_flags::none);

  if (r.has_value() && (EAGAIN == r.value())) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, "StableClient::sendMessage invalid response!");
    m_dockerCommandStatus = Q_COMMAND_EXECUTION_STATE::FAILED;
    return "";
  } else {
    QLogger::GetInstance().Log(LOGLEVEL::DBG2, "StableClient::sendMessage Received response: ", recv.to_string());

    if (recv.to_string().find(s_failedResponse) != std::string::npos) {
      m_dockerCommandStatus = Q_COMMAND_EXECUTION_STATE::FAILED;
      ErrorHandler::GetInstance().setError("Docker Command Failure",
                                           "Command:\n\n" + message + "\n\nResponse: \n\n" + recv.to_string());
    } else {
      m_dockerCommandStatus = Q_COMMAND_EXECUTION_STATE::SUCCESS;
      QLogger::GetInstance().Log(LOGLEVEL::DBG4, "Docker Command Response: ", recv.to_string());
    }

    return recv.to_string();
  }
}