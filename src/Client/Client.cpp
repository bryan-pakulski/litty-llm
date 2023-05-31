#include "Client/Client.h"
#include "Helpers/States.h"
#include "Helpers/QLogger.h"
#include <sstream>

Client::Client() {

  m_ctx = zmq::context_t{1};

  // Initialise general communication socket
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Client::Client Connecting main zmq interface", m_addr);
  m_socket = zmq::socket_t(m_ctx, zmq::socket_type::req);
  m_socket.connect(m_addr);

  // Initialise heartbeat socket for keepalive status
  QLogger::GetInstance().Log(LOGLEVEL::INFO, "Client::Client Connecting heartbeat zmq interface", m_heartbeat_addr);
  m_heartbeatSocket = zmq::socket_t(m_ctx, zmq::socket_type::req);
  m_heartbeatSocket.connect(m_heartbeat_addr);
  m_heartbeatSocket.set(zmq::sockopt::rcvtimeo, 5000);
}

Client::~Client() {}

// Heartbeat ping/pong to determine server status
void Client::heartbeat(int &state) {
  commands::heartbeat cmd = commands::heartbeat();
  zmq::message_t msg = zmq::message_t(cmd.getCommandString());
  zmq::message_t pong;

  try {
    m_heartbeatSocket.send(msg, zmq::send_flags::none);

    m_heartbeatSocket.recv(pong, zmq::recv_flags::none);
    state = HEARTBEAT_STATE::ALIVE;
  } catch (const zmq::error_t &err) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, err.what());
    state = HEARTBEAT_STATE::DEAD;

    // reset socket
    m_heartbeatSocket.disconnect(m_heartbeat_addr);
    m_heartbeatSocket = zmq::socket_t(m_ctx, zmq::socket_type::req);
    m_heartbeatSocket.connect(m_heartbeat_addr);

    return;
  }
}

// Reloads sd server in docker (release any models in memory)
void Client::releaseMemory(int &state) {
  commands::restartServer cmd = commands::restartServer();
  std::string msg = sendMessage(cmd.getCommandString(), state);
  state = Q_EXECUTION_STATE::SUCCESS;
}

// Load a stable diffusion model into memory in preperation for running inference commands
void Client::loadModelToMemory(commands::loadModelToMemory cmd, int &state) {
  std::string msg = sendMessage(cmd.getCommandString(), state);
  state = Q_EXECUTION_STATE::SUCCESS;
}

// Send message to ZMQ server listening on m_socket
std::string Client::sendMessage(const std::string &message, int &state) {
  std::lock_guard<std::mutex> guard(m_mutex);

  m_socket.send(zmq::buffer(message));

  zmq::message_t recv;
  m_socket.recv(recv, zmq::recv_flags::none);

  return recv.str();
}