#include <limits>

#include "our_gl.h"
#include "model.h"
#include "smooth_shader.h"

extern mat<4,4> ModelView, Perspective, Viewport;
extern std::vector<double> zbuffer;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " model.obj [diffuse.tga] [normal.tga]" << std::endl;
        return 1;
    }

    constexpr int width  = 800;   // 输出图像尺寸
    constexpr int height = 800;
    constexpr int shadoww = 8000; // 阴影贴图分辨率（越大阴影边缘越细）
    constexpr int shadowh = 8000;
    constexpr vec3  light{ 1, 1, 1}; // 光源方向/位置
    constexpr vec3    eye{-1, 0, 2}; // 相机位置
    constexpr vec3 center{ 0, 0, 0}; // 相机观察目标
    constexpr vec3     up{ 0, 1, 0}; // 相机上方向

    const std::string model_path = argv[1];
    const std::string diffuse_path = (argc > 2) ? argv[2] : "";
    const std::string normal_path = (argc > 3) ? argv[3] : "";
    const std::string floor_model_path = "obj/floor.obj";
    const std::string floor_diffuse_path = "obj/floor_diffuse.tga";
    const std::string floor_normal_path = "obj/floor_nm_tangent.tga";

    Model floor_model(floor_model_path, floor_diffuse_path, floor_normal_path); // 地板
    Model model(model_path, diffuse_path, normal_path);                          // 主模型

    // 第一阶段：从相机视角渲染无阴影结果
    lookat(eye, center, up);
    init_perspective(norm(eye-center));
    init_viewport(width/16, height/16, width*7/8, height*7/8);
    init_zbuffer(width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB);

    SmoothShader floor_shader(light, floor_model);
    for (int f = 0; f < floor_model.nfaces(); f++) {
        Triangle clip = {
            floor_shader.vertex(f, 0),
            floor_shader.vertex(f, 1),
            floor_shader.vertex(f, 2)
        };
        rasterize(clip, floor_shader, framebuffer, zbuffer);
    }

    SmoothShader head_shader(light, model);
    for (int f = 0; f < model.nfaces(); f++) {
        Triangle clip = {
            head_shader.vertex(f, 0),
            head_shader.vertex(f, 1),
            head_shader.vertex(f, 2)
        };
        rasterize(clip, head_shader, framebuffer, zbuffer);
    }

    const std::vector<double> camera_zbuffer = zbuffer;
    const mat<4,4> camera_inv = (Viewport * Perspective * ModelView).invert();

    // 第二阶段：从光源视角渲染深度图（shadow map）
    std::vector<double> shadow_zbuffer(shadoww * shadowh, -std::numeric_limits<double>::max());
    lookat(light, center, up);
    init_perspective(norm(light-center));
    init_viewport(0, 0, shadoww, shadowh);
    TGAImage shadowbuffer(shadoww, shadowh, TGAImage::RGB);

    DepthShader floor_depth_shader(floor_model);
    for (int f = 0; f < floor_model.nfaces(); f++) {
        Triangle clip = {
            floor_depth_shader.vertex(f, 0),
            floor_depth_shader.vertex(f, 1),
            floor_depth_shader.vertex(f, 2)
        };
        rasterize(clip, floor_depth_shader, shadowbuffer, shadow_zbuffer);
    }

    DepthShader head_depth_shader(model);
    for (int f = 0; f < model.nfaces(); f++) {
        Triangle clip = {
            head_depth_shader.vertex(f, 0),
            head_depth_shader.vertex(f, 1),
            head_depth_shader.vertex(f, 2)
        };
        rasterize(clip, head_depth_shader, shadowbuffer, shadow_zbuffer);
    }

    const mat<4,4> light_mvp = Viewport * Perspective * ModelView;

    // 第三阶段：后处理阴影判定与压暗（与 tinyrenderer-shadowmap 同思路）
    std::vector<bool> lit_mask(width * height, false);
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            const int idx = x + y * width;
            vec4 fragment = camera_inv * vec4{static_cast<double>(x), static_cast<double>(y), camera_zbuffer[idx], 1.0};
            vec4 q = light_mvp * fragment;
            vec3 p = q.xyz() / q.w;
            bool lit = (fragment.z < -100.0 ||
                        p.x < 0.0 || p.x >= shadoww || p.y < 0.0 || p.y >= shadowh ||
                        p.z > shadow_zbuffer[static_cast<int>(p.x) + static_cast<int>(p.y) * shadoww] - 0.03);
            lit_mask[idx] = lit;
        }
    }

    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            const int idx = x + y * width;
            if (lit_mask[idx]) continue;
            TGAColor c = framebuffer.get(x, y);
            vec3 a = {static_cast<double>(c[0]), static_cast<double>(c[1]), static_cast<double>(c[2])};
            if (norm(a) < 80.0) continue;
            a = normalized(a) * 80.0;
            framebuffer.set(x, y, {static_cast<unsigned char>(a[0]),
                                   static_cast<unsigned char>(a[1]),
                                   static_cast<unsigned char>(a[2]), 255});
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
