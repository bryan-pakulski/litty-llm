#pragma once

#include "Config/config.h"
#include "Helpers/QLogger.h"
#include "Helpers/GLHelper.h"
#include "Client/Heartbeat.h"
#include "Display/ErrorHandler.h"
#include "Display/QDisplay_Base.h"
#include "QDisplay_ConfigureModel.h"
#include "QDisplay_ImportModel.h"
#include "QDisplay_LoadModel.h"

#include <fstream>
#include <imgui.h>
#include <memory>

class QDisplay_TopBar : public QDisplay_Base {

private:
  // Window triggers
  bool logFileOpen = false;

  std::unique_ptr<QDisplay_ConfigureModel> m_configureModelWindow;
  std::unique_ptr<QDisplay_ImportModel> m_importModelWindow;
  std::unique_ptr<QDisplay_LoadModel> m_loadModelWindow;

  // Docker status icons
  std::unique_ptr<Image> m_docker_connected_icon;
  std::unique_ptr<Image> m_docker_disconnected_icon;
  float c_dockerIconSize = 15.0f;

  // Log config
  std::ifstream logStream;
  std::stringstream logFileBuffer;
  bool logUpdated = true;
  clock_t lastLogReadTime;

  // New file config
  char m_canvasName[256] = "new";

  /*
   * Popup for displaying log file output
   */
  void QDisplay_LogFile() {

    if (logFileOpen) {
      ImGui::SetNextWindowBgAlpha(0.9f);
      ImGui::SetNextWindowSize(ImVec2(CONFIG::IMGUI_LOG_WINDOW_WIDTH.get(), CONFIG::IMGUI_LOG_WINDOW_HEIGHT.get()));
      ImGui::Begin("Log");

      if (ImGui::Button("Close")) {
        logFileOpen = false;
      }

      ImGui::SameLine();

      if (ImGui::Button("Clear")) {
        logStream.close();
        QLogger::GetInstance().resetLog();
      }

      ImGui::Separator();

      ImGui::BeginChild("ScrollingLog");

      // Only update text if required
      if (QLogger::GetInstance().m_LAST_WRITE_TIME != lastLogReadTime) {
        logStream.open(QLOGGER_LOGFILE, std::ios::in);

        logFileBuffer.clear();
        logFileBuffer.str(std::string());

        lastLogReadTime = QLogger::GetInstance().m_LAST_WRITE_TIME;
        logFileBuffer << logStream.rdbuf();
        logStream.close();
        logUpdated = true;
      }

      ImGui::TextUnformatted(logFileBuffer.str().c_str());

      // Move to bottom of screen
      if (logUpdated) {
        ImGui::SetScrollY(ImGui::GetScrollMaxY() + ImGui::GetStyle().ItemSpacing.y * 2);
        logUpdated = false;
      }
      ImGui::EndChild();

      ImGui::End();
    }
  }

public:
  // Initialise render manager reference
  QDisplay_TopBar(GLFWwindow *w) : QDisplay_Base(w) {
    m_configureModelWindow = std::unique_ptr<QDisplay_ConfigureModel>(new QDisplay_ConfigureModel(w));
    m_importModelWindow = std::unique_ptr<QDisplay_ImportModel>(new QDisplay_ImportModel(w));
    m_loadModelWindow = std::unique_ptr<QDisplay_LoadModel>(new QDisplay_LoadModel(w));

    // Load images
    m_docker_connected_icon = std::unique_ptr<Image>(new Image(32, 32, "connected_icon"));
    m_docker_disconnected_icon = std::unique_ptr<Image>(new Image(32, 32, "disconnected_icon"));

    m_docker_connected_icon->loadFromImage("data/images/connected.png");
    m_docker_disconnected_icon->loadFromImage("data/images/disconnected.png");
  }

  /*
   * Main Menu renderer, contains logic for showing additional display items
   */
  virtual void render() {
    if (ImGui::BeginMainMenuBar()) {

      if (ImGui::BeginMenu("File")) {

        if (ImGui::MenuItem("Import Model")) {
          m_importModelWindow->openWindow();
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Models")) {

        if (ImGui::MenuItem("Load Model To Memory")) {
          m_loadModelWindow->openWindow();
        }

        if (ImGui::MenuItem("Configure Models")) {
          m_configureModelWindow->openWindow();
        }

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Debug")) {

        if (ImGui::MenuItem("Release model from memory")) {
          DockerCommandsInterface::GetInstance().releaseModelServer();
        }

        if (ImGui::MenuItem("Restart SD Server")) {
          DockerCommandsInterface::GetInstance().launchModelServer();
        }

        if (ImGui::MenuItem("Open Log")) {
          logFileOpen = true;
        }

        ImGui::EndMenu();
      }

      // Docker connection state
      ImGui::Separator();
      {
        Image icon = Heartbeat::GetInstance().getState() ? *m_docker_connected_icon : *m_docker_disconnected_icon;
        ImGui::Image((void *)(intptr_t)icon.m_texture, {c_dockerIconSize, c_dockerIconSize}, {1, 0}, {0, 1});
        std::string dockerState = Heartbeat::GetInstance().getState() ? "Docker Connected" : "Docker Disconnected";
        ImGui::MenuItem(dockerState.c_str());
      }

      ImGui::EndMainMenuBar();
    }

    // These will only render if their corresponding flags are set
    QDisplay_LogFile();

    // Render additional windows
    m_configureModelWindow->render();
    m_importModelWindow->render();
    m_loadModelWindow->render();
  }
};