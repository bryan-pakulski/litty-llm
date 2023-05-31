#pragma once

#include <fstream>
#include <imgui.h>

#include "GLFW/glfw3.h"
#include "Helpers/QLogger.h"

// Basic menu class
// QDisplay uses this base class as a reference via smart pointer, this is to
// call the overloaded virtual render function

struct listItem {
  std::string m_name;
  std::string m_key;
  bool m_isSelected = false;
};
class QDisplay_Base {

protected:
  GLFWwindow *m_window;

  void getWindowSize(std::pair<int, int> &size) { glfwGetFramebufferSize(m_window, &size.first, &size.second); }

public:
  bool m_isOpen = false;
  QDisplay_Base(GLFWwindow *w) : m_window(w) {}
  virtual void render() {}
};
