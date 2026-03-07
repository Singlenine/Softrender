#include <algorithm>
#include <cmath>
#include "smooth_shader.h"

namespace {
constexpr double kMinShadowLight = 0.35;
constexpr int kPcfRadius = 1;          // 1 -> 3x3 kernel
constexpr double kBiasMin = 0.001;
constexpr double kBiasSlope = 0.02;
}

DepthShader::DepthShader(const Model &m) : model(m) {}

vec4 DepthShader::vertex(const int face, const int vert) {
  vec4 gl_Position = ModelView * model.vert(face, vert);
  return Perspective * gl_Position;
}

std::pair<bool, TGAColor> DepthShader::fragment(const vec3) const {
  return {false, TGAColor{{255, 255, 255, 255}, 4}};
}

SmoothShader::SmoothShader(const vec3 light, const Model &m,
                           const mat<4,4> &shadow_mv, const mat<4,4> &shadow_p, const mat<4,4> &shadow_vp,
                           const std::vector<double> &shadow_buf, const int shadow_w, const int shadow_h)
    : model(m),
      shadow_modelview(shadow_mv),
      shadow_perspective(shadow_p),
      shadow_viewport(shadow_vp),
      shadow_zbuffer(shadow_buf),
      shadow_width(shadow_w),
      shadow_height(shadow_h) {
  l = normalized((ModelView * vec4{light.x, light.y, light.z, 0.}));
}

vec4 SmoothShader::vertex(const int face, const int vert) {
  varying_uv[vert] = model.uv(face, vert);
  const vec4 world_pos = model.vert(face, vert);
  varying_nrm[vert] = ModelView.invert_transpose() * model.normal(face, vert);
  vec4 camera_pos = ModelView * world_pos;
  tri[vert] = camera_pos.xyz() ;
  varying_shadow_clip[vert] = shadow_perspective * (shadow_modelview * world_pos);
  return Perspective * camera_pos;
}

std::pair<bool, TGAColor> SmoothShader::fragment(const vec3 bar) const {
  vec4 e0 = vec4{tri[1].x - tri[0].x, tri[1].y - tri[0].y, tri[1].z - tri[0].z, 0};
  vec4 e1 = vec4{tri[2].x - tri[0].x, tri[2].y - tri[0].y, tri[2].z - tri[0].z, 0};
  mat<2,4> E = {e0, e1};
  mat<2,2> U = {varying_uv[1] - varying_uv[0], varying_uv[2] - varying_uv[0]};
  mat<2,4> T = U.invert() * E;
  vec4 interpolated_nrm = varying_nrm[0] * bar[0] + varying_nrm[1] * bar[1] + varying_nrm[2] * bar[2];
  mat<4,4> D = {normalized(T[0]),
                normalized(T[1]),
                normalized(interpolated_nrm),
                {0, 0, 0, 1}};
  vec2 uv = varying_uv[0] * bar[0] + varying_uv[1] * bar[1] + varying_uv[2] * bar[2];
  TGAColor gl_FragColor = model.diffuse(uv);
  vec4 n = normalized(D.transpose() * model.normal(uv));
  vec4 r = normalized(n * (n * l) * 2 - l);
  double ambient = .4;
  double diffuse = 1. * std::max(0., n * l);
  double specular = 0.5 * std::pow(std::max(r.z, 0.), 35);

  vec4 shadow_clip = varying_shadow_clip[0] * bar[0] + varying_shadow_clip[1] * bar[1] + varying_shadow_clip[2] * bar[2];
  shadow_clip = shadow_clip / shadow_clip.w;
  vec4 shadow_screen = shadow_viewport * shadow_clip;
  int sx = std::clamp(static_cast<int>(shadow_screen.x), 0, shadow_width - 1);
  int sy = std::clamp(static_cast<int>(shadow_screen.y), 0, shadow_height - 1);
  double ndotl = std::max(0., n * l);
  double bias = std::max(kBiasMin, kBiasSlope * (1.0 - ndotl));

  double visibility = 0.0;
  int samples = 0;
  for (int ox = -kPcfRadius; ox <= kPcfRadius; ++ox) {
    for (int oy = -kPcfRadius; oy <= kPcfRadius; ++oy) {
      int px = std::clamp(sx + ox, 0, shadow_width - 1);
      int py = std::clamp(sy + oy, 0, shadow_height - 1);
      double nearest = shadow_zbuffer[px + py * shadow_width];
      visibility += (shadow_screen.z <= nearest + bias) ? 1.0 : 0.0;
      samples++;
    }
  }
  visibility /= samples;
  double shadow = kMinShadowLight + (1.0 - kMinShadowLight) * visibility;

  for (int channel : {0, 1, 2})
    gl_FragColor[channel] = std::min<int>(255, gl_FragColor[channel] * (ambient + shadow * (diffuse + specular)));
  return {false, gl_FragColor};
}

