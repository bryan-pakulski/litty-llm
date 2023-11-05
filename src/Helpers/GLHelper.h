#pragma once
#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "Helpers/QLogger.h"
#include "Helpers/States.h"

#include "ThirdParty/stb/stb_image.h"
#include "ThirdParty/stb/stb_image_write.h"

class GLHELPER {
public:
  static bool LoadTextureFromFile(const char *filename, GLuint *out_texture, int *out_width, int *out_height,
                                  bool tiled, bool flipImage);

  static void SaveTextureToFile(const char *filename, GLuint *texture, int width, int height);
};

class Image {

public:
  GLuint m_texture = 0;
  std::string m_name;
  std::string m_image_source;

  int m_width;
  int m_height;

  Image(int width, int height, std::string name) : m_name(name), m_width(width), m_height(height) {
    float color[] = {0.0f, 0.0f, 0.0f, 1.0f};

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, color);
    glBindTexture(GL_TEXTURE_2D, 0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    QLogger::GetInstance().Log(LOGLEVEL::DBG2, "Image::Image Successfully created blank texture", m_name);
  }
  ~Image() = default;

  void loadFromImage(std::string path, bool flipImage = false) {
    if (!GLHELPER::LoadTextureFromFile(path.c_str(), &m_texture, &m_width, &m_height, false, flipImage)) {
      QLogger::GetInstance().Log(LOGLEVEL::ERR, "Image::loadFromImage Failed to load image from file: ", path.c_str());
    }
    m_image_source = path;
  }
};