#ifndef SMOOTH_SHADER_H
#define SMOOTH_SHADER_H

#include "our_gl.h"
#include "model.h"
#include "geometry.h"
#include "tgaimage.h"

struct SmoothShader : IShader
{
  const Model &model;
  vec4 l; // light direction in eye coordinates
  mat<3,3> tri;//triangle coordinates before  viewport transform
  mat<3,3> normals;// triangle normal vectors befor viewport transform
  vec2 varying_uv[3]; // triangle uv coordinates, written by the vertex shader, read by the fragment shader
  vec4 varying_nrm[3]; //normal per vertex to be interpolated by the fragment shader

  SmoothShader(const vec3 light, const Model &m);
  vec4 vertex(const int face, const int vert);
  std::pair<bool, TGAColor> fragment(const vec3 bar) const override;
};


#endif
