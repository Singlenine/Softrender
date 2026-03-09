# SoftRender: CPU 软光栅渲染器

一个基于 C++20 的软件渲染器，不依赖 OpenGL/DirectX 光栅化管线，在 CPU 上完整实现模型加载、变换、光栅化、深度测试、法线贴图与阴影映射流程。

## 项目目标

- 复现经典图形学渲染管线的核心步骤，强化对 MVP 变换与片元阶段的理解。
- 在可控代码规模下验证光照、法线扰动与阴影算法的工程实现细节。
- 形成可扩展的“迷你渲染框架”，便于后续增加 BRDF、后处理和多光源等功能。

## 主要功能

- OBJ 模型解析（顶点/法线/纹理坐标，三角面）
- TGA 纹理读取与采样（漫反射贴图、法线贴图）
- 两遍渲染（Two-Pass）阴影映射
  - Pass 1：从光源视角生成深度图
  - Pass 2：从相机视角着色并进行阴影查询
- Blinn-Phong 风格光照项
  - Ambient + Diffuse + Specular
- 切线空间法线贴图（由 UV 与几何边构建局部基）
- 深度缓冲（Z-buffer）与背面剔除
- PCF（3x3）软阴影滤波与 bias 抑制阴影痤疮
- OpenMP 并行光栅化（可选）

## 渲染流程概览

1. 加载主模型与地面模型（`obj/floor.obj`）。
2. 设置光源视角矩阵，渲染阴影深度缓冲。
3. 切换到相机视角，初始化颜色缓冲与 Z-buffer。
4. 对每个三角形执行：
   - 顶点着色：坐标变换、法线/UV/阴影坐标插值量写入
   - 光栅化：重心坐标插值，深度测试
   - 片元着色：法线贴图扰动 + 光照计算 + 阴影可见性（PCF）
5. 输出 `framebuffer.tga`。

## 代码结构

- `main.cpp`：程序入口，组织 two-pass 渲染流程。
- `our_gl.h/.cpp`：简化版图形 API（`lookat`、投影、viewport、光栅化、Z-buffer）。
- `smooth_shader.h / smoothshader.cpp`：
  - `DepthShader`：阴影图生成
  - `SmoothShader`：主通道着色（法线贴图 + 阴影）
- `model.h/.cpp`：OBJ/TGA 加载与纹理采样。
- `geometry.h`：向量/矩阵模板与线性代数操作。
- `tgaimage.h/.cpp`：TGA 图像读写。
- `obj/`：示例模型与贴图资源。

## 环境要求

- CMake `3.12+`
- 支持 C++20 的编译器（MSVC / GCC / Clang）
- 可选：OpenMP（未检测到时自动关闭并行）

## 构建与运行

### Windows (PowerShell)

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
.\build\Release\tinyrenderer.exe .\obj\african_head\african_head.obj .\obj\african_head\african_head_diffuse.tga .\obj\african_head\african_head_nm.tga
```


程序参数说明：

```text
tinyrenderer model.obj [diffuse.tga] [normal.tga]
```

- `model.obj`：必选
- `diffuse.tga`：可选，不提供时默认白色漫反射
- `normal.tga`：可选，不提供时使用几何法线

运行完成后会在项目根目录输出：

- `framebuffer.tga`：最终渲染结果

## 关键实现说明

- 阴影质量：使用 `kPcfRadius = 1`（3x3 核）做 PCF，兼顾质量与性能。
- 阴影稳定性：采用基于法线与光线夹角的 slope-scale bias（`kBiasMin`, `kBiasSlope`）减少阴影痤疮。
- 可见性底限：`kMinShadowLight` 避免阴影区域过黑，提升视觉层次。
- 并行化：在包围盒扫描阶段通过 `#pragma omp parallel for` 并行处理像素列。

## 当前限制

- 仅支持三角化 OBJ（非三角面会报错退出）。
- 未实现透视校正插值（当前深度与属性插值为线性形式）。
- 未实现纹理 Mipmap/各向异性过滤。
- 单光源、单相机路径，缺少场景管理层。

## 后续改进方向

1. 增加透视校正插值与更严格的数值稳定处理。
2. 引入多光源与可插拔 BRDF（Lambert/Blinn-Phong/PBR）。
3. 增加后处理模块（Gamma/Tone Mapping/FXAA）。
4. 拆分渲染管线阶段，补齐单元测试与性能基准。

## 致谢

项目整体思路参考 TinyRenderer 教学路线，并在此基础上加入了阴影映射、PCF 与法线贴图等可视化效果增强实现。
