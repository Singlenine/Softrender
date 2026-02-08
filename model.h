#pragma once

#include <vector>
#include "geometry.h"

class Model {
    std::vector<vec3> verts = {};    // array of vertices
    std::vector<vec3> norms = {};    // 顶点法线
    std::vector<int> facet_vrt = {}; // per-triangle index in the above array
    std::vector<int> facet_nrm = {}; // 纹理索引
public:
    Model(const std::string filename);
    int nverts() const; // number of vertices
    int nfaces() const; // number of triangles
    int nnormals() const; //法线数
    vec3 vert(const int i) const;                          // 0 <= i < nverts()
    vec3 vert(const int iface, const int nthvert) const;   // 0 <= iface <= nfaces(), 0 <= nthvert < 3
    vec3 normal(const int iface, const int nthvert) const; // 0 <= iface <= nfaces(), 0 <= nthvert < 3
};

