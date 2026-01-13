# SoftRender - 软件渲染器

一个从零开始实现的C++软件3D渲染器，不依赖GPU，展示了计算机图形学的核心算法原理。

## 项目概述

SoftRender是一个纯软件实现的3D渲染引擎，通过实现图形学的基础算法，从而能够将3D模型渲染到2D图像。项目特点包括：

- 纯CPU渲染，不依赖GPU或图形API
- 实现了完整的3D渲染管线基础功能
- 支持OBJ格式模型和TGA格式纹理
- 包含基础的光照计算模型
- 深度测试和隐面消除

## 项目结构

- `main.cpp` - 主渲染循环和核心算法实现
- `tgaimage.cpp/h` - TGA图像格式处理库
- `model.cpp/h` - 3D模型加载和处理
- `geometry.h` - 向量和矩阵数学库
- `obj/` - 存放3D模型文件
- `output.tga` - 渲染输出结果

## 核心算法实现

### 1. 绘图基础算法

#### 线段绘制 (Bresenham中点算法)
```cpp
void line(Vec2i t0, Vec2i t1, TGAImage &image, TGAColor color) { 
    // 改进的Bresenham中点算法实现
    // 处理不同斜率、方向的直线绘制
}
```

#### 三角形填充 (扫描线算法)
```cpp
void triangle0(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color) {
    // 扫描线算法实现，逐行填充三角形
}
```

### 2. 几何处理

#### 重心坐标计算
```cpp
Vec3f barycentric(Vec3f *pts, Vec3f P) {
    // 使用叉积计算重心坐标
    // 用于判断点是否在三角形内部并进行属性插值
}
```

#### 世界坐标到屏幕坐标转换
```cpp
Vec3f world2screen(Vec3f v) {
    // 3D坐标转换为屏幕坐标
}
```

### 3. 纹理映射与光照

#### UV坐标计算
```cpp
std::pair<int,int> computeUV(float u, float v, TGAImage* textureTga) {
    // 计算纹理映射坐标
}
```

#### 三角形渲染 (包含纹理和光照)
```cpp
void triangle(std::vector<std::pair<int,int>>& face, Model* model, 
              TGAImage* textureTga, float* zbuffer, TGAImage& image, 
              float intensity) {
    // 带纹理映射和光照计算的三角形渲染
}
```

### 4. 深度测试 (Z-Buffer算法)

在三角形渲染过程中，使用Z-Buffer来处理物体的前后遮挡关系：

```cpp
if (zbuffer[int(P.x+P.y*width)] < P.z) {
    zbuffer[int(P.x+P.y*width)] = P.z;
    image.set(P.x, P.y, color);
}
```

## 渲染流程

1. **模型加载**：从OBJ文件加载3D模型数据
2. **顶点处理**：将世界坐标转换到屏幕坐标
3. **光照计算**：计算每个三角形的光照强度
4. **三角形绘制**：
   - 计算三角形的包围盒
   - 对包围盒内的每个像素进行测试
   - 使用重心坐标判断像素是否在三角形内
   - 进行深度测试
   - 计算纹理坐标并应用纹理
   - 应用光照
   - 写入最终像素颜色
5. **输出图像**：将渲染结果保存为TGA格式图像

## 编译与运行

### 直接编译

```bash
g++ -std=c++17 -O3 model.cpp tgaimage.cpp main.cpp -o main
```

### 运行

```bash
./main [model_file] [texture_file]
```

- 不指定参数时，默认使用`obj/african_head.obj`和对应纹理
- 指定一个参数时，加载指定模型但不使用纹理
- 指定两个参数时，加载指定模型和纹理

## 算法性能优化

- **包围盒优化**：只对三角形包围盒内的像素进行处理
- **边界检查优化**：确保坐标在有效屏幕范围内
- **向量运算优化**：使用高效的向量数学库

## 未来改进方向

- 实现更多高级渲染特性（法线贴图、阴影等）
- 优化渲染性能
- 支持更复杂的光照模型
- 添加动画支持
- 实现更多几何处理算法

## 参考资料

- 《计算机图形学》相关教材
- ssloy/tinyrenderer项目
- 渲染管线理论

## 效果展示

执行渲染后，将在项目目录生成`output.tga`文件，可使用任何支持TGA格式的图像查看器打开。

![渲染结果](output.tga)

---

*注：本项目仅用于学习计算机图形学基础原理，展示了软件渲染的核心算法实现。* 