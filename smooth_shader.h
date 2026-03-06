#ifndef SMOOTH_SHADER_H
#define SMOOTH_SHADER_H

#include "our_gl.h"
#include "model.h"
#include "geometry.h"
#include "tgaimage.h"

struct SmoothShader : IShader
{
  const Model &model;
  vec4 l;
  mat<3,3> tri;//三角形顶点
  mat<3,3> normals;// 三角形顶点法线
  vec2 varying_uv[3];; //顶点纹理坐标

  SmoothShader(const vec3 light, const Model &m ) : model(m) {
    l = normalized((ModelView*vec4{light.x, light.y, light.z, 0.})); // transform the light vector to view coordinates
  }
  
  vec4 vertex(const int face, const int vert) {
    varying_uv[vert] = model.uv(face, vert);
    vec4 gl_Position = ModelView * model.vert(face, vert);
    return Perspective * gl_Position;                         // in clip coordinates
  }

  virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const override {
   /* TGAColor result = {255, 255, 255, 255};
    vec3 n = normalized(normals.transpose() * bar);
    vec2 uv = varying_uv * bar;
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
    return {false, result};平滑渲染源代码*/
    vec2 uv = varying_uv[0] * bar[0] + varying_uv[1] * bar[1] + varying_uv[2] * bar[2];
    TGAColor gl_FragColor = model.diffuse(uv);                   // base color from diffuse map
    vec4 n = normalized(ModelView.invert_transpose() * model.normal(uv));
    vec4 r = normalized(n * (n * l)*2 - l);                      // reflected light direction
    double ambient = .3;                                          // ambient light intensity
    double diff = std::max(0., n * l);                            // diffuse light intensity
    double spec = std::pow(std::max(r.z, 0.), 35);                // specular intensity
    double intensity = std::min(1., ambient + .4*diff + .9*spec);
    for (int channel : {0,1,2})
        gl_FragColor[channel] = static_cast<std::uint8_t>(gl_FragColor[channel] * intensity);
    return {false, gl_FragColor};                
}

};


#endif
