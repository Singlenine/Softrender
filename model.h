#pragma once
#include "geometry.h"
#include "tgaimage.h"

class Model {
    std::vector<vec4> verts = {};    // 顶点数组（通常与下方数组长度不一致）
    std::vector<vec4> norms = {};    // 法线数组
    std::vector<vec2> tex = {};      // 纹理坐标数组
    std::vector<int> facet_vrt = {}; // 每个三角面在顶点数组中的索引，理论长度为 nfaces()*3
    std::vector<int> facet_nrm = {}; // 每个三角面在法线数组中的索引，理论长度为 nfaces()*3
    std::vector<int> facet_tex = {}; // 每个三角面在纹理坐标数组中的索引，理论长度为 nfaces()*3
    TGAImage diffusemap  = {};       // 漫反射贴图
    TGAImage normalmap   = {};       // 法线贴图
public:
    Model(const std::string filename, const std::string diffuse_tex = "", const std::string normal_tex = "");
    int nverts() const; // 顶点数量
    int nfaces() const; // 三角面数量
    vec4 vert(const int i) const;                          // 0 <= i < nverts()
    vec4 vert(const int iface, const int nthvert) const;   // 0 <= iface <= nfaces(), 0 <= nthvert < 3
    vec4 normal(const int iface, const int nthvert) const; // 来自 .obj 文件中 "vn x y z" 记录的法线
    vec4 normal(const vec2 &uv) const;                     // 从法线贴图采样得到的法线向量
    TGAColor diffuse(const vec2 &uv) const;                // 从漫反射贴图采样得到的颜色
    vec2 uv(const int iface, const int nthvert) const;     // 三角形顶点的 uv 坐标
};
