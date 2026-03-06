#include <algorithm>
#include <cmath>
#include "smooth_shader.h"

SmoothShader::SmoothShader(const vec3 light, const Model &m) : model(m) {
  l = normalized((ModelView * vec4{light.x, light.y, light.z, 0.}));
}

vec4 SmoothShader::vertex(const int face, const int vert) {
  varying_uv[vert] = model.uv(face, vert);
  varying_nrm[vert] = ModelView.invert_transpose() * model.normal(face, vert);
  vec4 gl_Position = ModelView * model.vert(face, vert);
  tri[vert] = gl_Position.xyz() ;
  return Perspective * gl_Position;
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
  for (int channel : {0, 1, 2})
    gl_FragColor[channel] = std::min<int>(255, gl_FragColor[channel] * (ambient + diffuse + specular));
  return {false, gl_FragColor};
}

