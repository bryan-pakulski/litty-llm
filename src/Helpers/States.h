#pragma once

#include <string>

#ifndef _EXEC_STATE
#define _EXEC_STATE

// Docker command execution
struct Q_EXECUTION_STATE {
  const static int FAILED = -1;
  const static int PENDING = 0;
  const static int SUCCESS = 1;
};

// Heartbeat status

struct HEARTBEAT_STATE {
  const static int DEAD = -1;
  const static int POLL = 0;
  const static int ALIVE = 1;
};

#endif
