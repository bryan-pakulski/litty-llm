#include "LLMManager.h"
#include "Client/Heartbeat.h"
#include "Display/ErrorHandler.h"
#include "Display/QDisplay.h"
#include <imgui_impl_glfw.h>
#include <memory>

int main() {

  // Initialise heartbeat to docker
  Heartbeat::GetInstance().start();
  LLMManager::GetInstance().launchLLMModelServer();

  while (!glfwWindowShouldClose(QDisplay::GetInstance().getWindow())) {

    // Clean OpenGL frame & imgui interface
    QDisplay::clearFrame();

    // Only render if no errors detected
    if (!ErrorHandler::GetInstance().hasError()) {
      // Sub menus rendering & logic
      QDisplay::GetInstance().drawMenus();
    }

    // Display any captured errors as a modal popup over the top of the screen
    ErrorHandler::GetInstance().pollErrors();

    // Process and catch events
    QDisplay::processFrameAndEvents();
  }

  Heartbeat::GetInstance().stop();

  return 0;
}
