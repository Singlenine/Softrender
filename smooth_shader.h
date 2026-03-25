#ifndef SMOOTH_SHADER_H
#define SMOOTH_SHADER_H

#include <vector>

#include "our_gl.h"
#include "model.h"
#include "geometry.h"
#include "tgaimage.h"

struct DepthShader : IShader {
  const Model &model;

  explicit DepthShader(const Model &m);
  vec4 vertex(const int face, const int vert);
  std::pair<bool, TGAColor> fragment(const vec3 bar) const override;
};

struct SmoothShader : IShader {
  const Model &model;
  vec4 l;              // light direction in view space
  mat<3,3> tri;        // triangle vertices in view space
  vec2 varying_uv[3];  // per-vertex uv for interpolation
  vec4 varying_nrm[3]; // per-vertex normal in view space

  SmoothShader(const vec3 light, const Model &m);
  vec4 vertex(const int face, const int vert);
  std::pair<bool, TGAColor> fragment(const vec3 bar) const override;
};

#endif
