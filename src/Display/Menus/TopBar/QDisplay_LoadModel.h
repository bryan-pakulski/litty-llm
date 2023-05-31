#pragma once

#include "Client/DockerCommandsInterface.h"
#include "Display/ErrorHandler.h"
#include "Display/QDisplay_Base.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <filesystem>

class QDisplay_LoadModel : public QDisplay_Base {

private:
  std::vector<listItem> m_ModelList;

  std::string m_selected_model = "";
  std::string m_selected_hash = "";

  void clear() {
    m_ModelList.clear();
    m_selected_model = "";
    m_selected_hash = "";
  }

  void reloadFiles() {
    // Load models list
    try {
      static YAML::Node configFile = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get());
      YAML::Node models = configFile["models"];
      for (YAML::const_iterator it = models.begin(); it != models.end(); ++it) {
        if (it->second["name"].as<std::string>() != "default") {
          listItem i{.m_name = it->second["name"].as<std::string>(), .m_key = it->first.as<std::string>()};
          m_ModelList.push_back(i);
        }
      }
    } catch (const YAML::Exception &) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "QDisplay_LoadModel::reloadFiles Failed to parse yaml file: ",
                                 CONFIG::MODELS_CONFIGURATION_FILE.get());
      return;
    }
  }

  // Attempt to load model
  void loadModel() {
    // Retrieve root node and dump back to file
    YAML::Node node, _baseNode = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get());
    YAML::Node model = _baseNode["models"][m_selected_hash];

    // TODO: get all extra configuration

    DockerCommandsInterface::GetInstance().attachModelToServer(model["path"].as<std::string>());

    m_isOpen = false;
  }

public:
  QDisplay_LoadModel(GLFWwindow *w) : QDisplay_Base(w) {}

  void openWindow() {
    clear();
    reloadFiles();
    m_isOpen = true;
  }

  virtual void render() {
    if (m_isOpen) {
      ImGui::Begin("Load Model");

      if (ImGui::BeginCombo("model config", m_selected_model.c_str(), ImGuiComboFlags_NoArrowButton)) {
        for (auto &item : m_ModelList) {
          if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
            m_selected_model = item.m_name;
            m_selected_hash = item.m_key;
          }
          if (item.m_isSelected) {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }

      // TODO: configuration items go here

      if (m_selected_hash != "default" && m_selected_hash != "") {
        if (ImGui::Button("Load Model")) {
          loadModel();
        }
        ImGui::SameLine();
      }

      if (ImGui::Button("Cancel")) {
        m_isOpen = false;
      }

      ImGui::End();
    }
  }
};