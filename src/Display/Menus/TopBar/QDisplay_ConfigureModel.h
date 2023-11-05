#pragma once

#include <imgui.h>
#include <filesystem>

#include "Config/model_config.h"
#include "Config/structs.h"
#include "Display/ErrorHandler.h"
#include "Display/QDisplay_Base.h"
#include <imgui_stdlib.h>

#include "yaml-cpp/emittermanip.h"
#include "yaml-cpp/node/detail/iterator_fwd.h"
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>

namespace fs = std::filesystem;

class QDisplay_ConfigureModel : public QDisplay_Base {
public:
  QDisplay_ConfigureModel(GLFWwindow *w) : QDisplay_Base(w) {}

  void openWindow() {
    m_isOpen = true;
    reloadFiles();
  };

  virtual void render() {
    // Render configure modules popup
    configureModelPopup();
  }

private:
  ModelConfig m_modelConfig;
  std::vector<listItem> m_ModelList;

private:
  void clear() {
    m_modelConfig = ModelConfig{};
    m_ModelList.clear();
  }

  void reloadFiles() {
    try {
      static YAML::Node configFile = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get());
      YAML::Node models = configFile["models"];
      for (YAML::const_iterator it = models.begin(); it != models.end(); ++it) {
        listItem i{.m_name = it->second["name"].as<std::string>(), .m_key = it->first.as<std::string>()};
        m_ModelList.push_back(i);
      }
    } catch (const YAML::Exception &err) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "QDisplay_ConfigureModel::reloadFiles Failed to parse yaml file: ",
                                 CONFIG::MODELS_CONFIGURATION_FILE.get(), err.what());
      return;
    }
  }

  void configureModelPopup() {
    if (m_isOpen) {
      ImGui::Begin("Configure Models");

      ImGui::Text("Select a model to configure");
      if (ImGui::BeginCombo("model", m_modelConfig.name.c_str(), ImGuiComboFlags_NoArrowButton)) {
        for (auto &item : m_ModelList) {
          if (ImGui::Selectable(item.m_name.c_str(), item.m_isSelected)) {
            m_modelConfig = MODEL_CONFIG::loadModelConfig(item.m_key);
          }
          if (item.m_isSelected) {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }

      if (m_modelConfig.name != "") {

        ImGui::Text("Optimisations");
        ImGui::Separator();

        // TODO: mouse over popups with context information
        ImGui::Checkbox("Use CPU", &m_modelConfig.use_cpu);

        // Save available once we fill in mandatory information
        if (m_modelConfig.model_path != "") {
          if (ImGui::Button("Save")) {
            m_isOpen = false;
            MODEL_CONFIG::saveModelConfig(m_modelConfig);
            clear();
          }
          ImGui::SameLine();
        }
      }

      if (ImGui::Button("Cancel")) {
        m_isOpen = false;
        clear();
      }
      ImGui::End();
    }
  }
};