#pragma once

#include "Display/ErrorHandler.h"
#include "Display/QDisplay_Base.h"
#include "Helpers/hash.h"

#include <imgui.h>
#include "ThirdParty/imgui/imfilebrowser.h"
#include <imgui_stdlib.h>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>
#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

class QDisplay_ImportModel : public QDisplay_Base {

private:
  ImGui::FileBrowser fileDialog;
  bool shared_bool = false;
  std::shared_ptr<bool> m_saving = std::make_shared<bool>(shared_bool);

  std::vector<listItem> m_ModelConfigList;

  void clear() { fileDialog.ClearSelected(); }

  // Save model configuration to model config file
  static void saveModelConfiguration(std::string filepath, std::shared_ptr<bool> m_saving) {
    std::filesystem::path modelPath(filepath);

    // Build yaml node to attach to model configuration file
    YAML::Node model_node;
    std::string hash = getFileHash(filepath.c_str());

    model_node["name"] = modelPath.filename().string();
    model_node["path"] = "/models/" + modelPath.filename().string();

    // Retrieve root node and dump back to file
    YAML::Node node, _baseNode = YAML::LoadFile(CONFIG::MODELS_CONFIGURATION_FILE.get());
    _baseNode["models"][hash] = model_node;
    std::ofstream fout(CONFIG::MODELS_CONFIGURATION_FILE.get());
    fout << _baseNode;

    // Copy model file
    try {
      fs::copy_file(modelPath, "data/models/" + modelPath.filename().string());
    } catch (fs::filesystem_error const &err) {
      ErrorHandler::GetInstance().setError(err.what());
    }

    *m_saving = false;
  }

  void configureModelPopup() {
    ImGui::Begin("Configure Model");

    ImGui::Text("%s", fileDialog.GetSelected().c_str());

    // TODO: populate model options here
    ImGui::Text("More model configuration options here...");

    if (ImGui::Button("Cancel")) {
      clear();
    } else if (!*m_saving) {
      ImGui::SameLine();
      if (ImGui::Button("Save")) {
        // Save on a seperate thread so we don't block application, use the m_saving shared pointer as a basic lock to
        // track completion
        *m_saving = true;
        std::thread t(saveModelConfiguration, fileDialog.GetSelected().string(), m_saving);
        t.detach();
        clear();
      }
    }

    ImGui::End();
  }

public:
  void openWindow() { fileDialog.Open(); };

  QDisplay_ImportModel(GLFWwindow *w) : QDisplay_Base(w) {
    fileDialog.SetTitle("Import Models");
    fileDialog.SetTypeFilters({".bin"});

    *m_saving = false;
  }

  virtual void render() {
    // File browser management
    fileDialog.Display();
    if (fileDialog.HasSelected()) {
      configureModelPopup();
    }

    if (*m_saving) {
      ImGui::Begin("Saving Model Configuration");
      ImGui::Text("Saving model configuration and copying to docker image, please wait...");
      ImGui::End();
    }
  }
};