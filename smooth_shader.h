#ifndef SMOOTH_SHADER_H
#define SMOOTH_SHADER_H

#include "our_gl.h"
#include "model.h"
#include "geometry.h"
#include "tgaimage.h"

struct SmoothShader : IShader
{
  const Model &model;
  vec3 l;
  mat<3,3> tri;//三角形顶点
  mat<3,3> normals;// 三角形顶点法线

  SmoothShader(const vec3 light, const Model &m) : model(m) {
    l = normalized((ModelView*vec4{light.x, light.y, light.z, 0.}).xyz());
  }
  
  vec4 vertex(const int face, const int vert) {
    vec3 v = model.vert(face, vert);
    vec4 pos =ModelView * vec4{v.x,v.y,v.z,1.};
    tri[vert] = pos.xyz();
    vec3 n = model.normal(face, vert);
    normals[vert] = normalized((ModelView * vec4{n.x,n.y,n.z,0.}).xyz());
    return Perspective * pos;
  }

  virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const override {
    TGAColor result = {255, 255, 255, 255};
    vec3 n = normalized(normals.transpose() * bar);
    double dot_nl = n * l; 
    double diff = std::max(0.0, dot_nl);
    vec3 r = normalized(n * (n * l) * 2 - l);
    double spec = std::pow(std::max(r.z, 0.), 35);
    double ambient = 0.3;
    double intensity = ambient + 0.4 * diff + 0.9 * spec;
    for (int channel : {0, 1, 2}) {
        double multiplier = std::min(1.0, intensity);
        result[channel] = static_cast<uint8_t>(result[channel] * multiplier);
    }
    return {false, result};
}

};


#endif