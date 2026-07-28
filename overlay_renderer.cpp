#include "engine/overlay_renderer.h"

#include "engine/log.h"

namespace unboundmp::android_engine {

namespace {

const char* kVertexShaderSrc = R"(
attribute vec2 a_position;
void main() {
  gl_Position = vec4(a_position, 0.0, 1.0);
}
)";

const char* kFragmentShaderSrc = R"(
precision mediump float;
uniform vec4 u_color;
void main() {
  gl_FragColor = u_color;
}
)";

GLuint CompileShader(GLenum type, const char* source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  GLint compiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    char log[512];
    glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
    UMP_LOGE("Overlay shader compile failed: %s", log);
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

// Converts a normalized (top-left origin, y-down) TouchRegion rect into
// six GL clip-space vertices (two triangles), flipping y since GL clip
// space is y-up.
void RegionToClipSpaceTriangles(const input::TouchRegion& region, GLfloat out[12]) {
  const float left = region.x * 2.0f - 1.0f;
  const float right = (region.x + region.width) * 2.0f - 1.0f;
  const float top = 1.0f - region.y * 2.0f;
  const float bottom = 1.0f - (region.y + region.height) * 2.0f;

  const GLfloat verts[12] = {
      left,  top,     left,  bottom, right, bottom,  // triangle 1
      left,  top,     right, bottom, right, top,     // triangle 2
  };
  for (int i = 0; i < 12; ++i) out[i] = verts[i];
}

}  // namespace

OverlayRenderer::~OverlayRenderer() { Shutdown(); }

bool OverlayRenderer::EnsureInitialized() {
  if (initialized_) return true;

  GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, kVertexShaderSrc);
  if (vertex_shader == 0) return false;
  GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, kFragmentShaderSrc);
  if (fragment_shader == 0) {
    glDeleteShader(vertex_shader);
    return false;
  }

  program_ = glCreateProgram();
  glAttachShader(program_, vertex_shader);
  glAttachShader(program_, fragment_shader);
  glLinkProgram(program_);

  GLint linked = 0;
  glGetProgramiv(program_, GL_LINK_STATUS, &linked);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  if (!linked) {
    char log[512];
    glGetProgramInfoLog(program_, sizeof(log), nullptr, log);
    UMP_LOGE("Overlay shader link failed: %s", log);
    glDeleteProgram(program_);
    program_ = 0;
    return false;
  }

  position_attrib_ = glGetAttribLocation(program_, "a_position");
  color_uniform_ = glGetUniformLocation(program_, "u_color");
  initialized_ = true;
  UMP_LOGI("Overlay renderer initialized");
  return true;
}

void OverlayRenderer::Draw(const std::vector<input::TouchRegion>& layout, uint32_t held_mask,
                            int32_t /*width*/, int32_t /*height*/) {
  if (!initialized_) return;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgram(program_);
  glEnableVertexAttribArray(static_cast<GLuint>(position_attrib_));

  for (const auto& region : layout) {
    GLfloat verts[12];
    RegionToClipSpaceTriangles(region, verts);
    glVertexAttribPointer(static_cast<GLuint>(position_attrib_), 2, GL_FLOAT, GL_FALSE, 0, verts);

    const bool held = (held_mask & emulator::InputState::BitFor(region.button)) != 0;
    // Held buttons render brighter/more opaque; unheld ones are a faint
    // outline-ish translucent fill - enough to see where the layout is
    // without obscuring a future game-video-output layer underneath it.
    if (held) {
      glUniform4f(color_uniform_, 0.9f, 0.9f, 1.0f, 0.55f);
    } else {
      glUniform4f(color_uniform_, 0.8f, 0.8f, 0.9f, 0.18f);
    }
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  glDisableVertexAttribArray(static_cast<GLuint>(position_attrib_));
  glDisable(GL_BLEND);
}

void OverlayRenderer::Shutdown() {
  if (program_ != 0) {
    glDeleteProgram(program_);
    program_ = 0;
  }
  initialized_ = false;
}

}  // namespace unboundmp::android_engine
