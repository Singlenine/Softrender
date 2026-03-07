#include <limits>

#include "our_gl.h"
#include "model.h"
#include "smooth_shader.h"

extern mat<4,4> ModelView, Perspective, Viewport; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;               // the depth buffer

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " model.obj [diffuse.tga] [normal.tga]" << std::endl;
        return 1;
    }

    constexpr int width  = 800;      // output image size
    constexpr int height = 800;
    constexpr int shadoww = 8000;    // shadow map buffer size
    constexpr int shadowh = 8000;
    constexpr vec3  light{ 1, 1, 1}; // light source
    constexpr vec3    eye{2, 3, 8};  // camera position
    constexpr vec3 center{ 0, 0, 0}; // camera direction
    constexpr vec3     up{ 0, 1, 0}; // camera up vector

    std::vector<double> shadow_zbuffer(shadoww * shadowh, -std::numeric_limits<double>::max());

    const std::string model_path = argv[1];
    const std::string diffuse_path = (argc > 2) ? argv[2] : "";
    const std::string normal_path = (argc > 3) ? argv[3] : "";
    const std::string floor_model_path = "obj/floor.obj";
    const std::string floor_diffuse_path = "obj/floor_diffuse.tga";
    const std::string floor_normal_path = "obj/floor_nm_tangent.tga";

    Model floor_model(floor_model_path, floor_diffuse_path, floor_normal_path); // floor model
    Model model(model_path, diffuse_path, normal_path); // head model

    // Pass 1: render depth from the light point of view.
    lookat(light, center, up);
    init_perspective(norm(light-center));
    init_viewport(0, 0, shadoww, shadowh);
    mat<4,4> shadow_modelview = ModelView;
    mat<4,4> shadow_perspective = Perspective;
    mat<4,4> shadow_viewport = Viewport;
    TGAImage shadowbuffer(shadoww, shadowh, TGAImage::RGB);

    DepthShader floor_depth_shader(floor_model);
    for (int f=0; f<floor_model.nfaces(); f++) {
        Triangle clip = { floor_depth_shader.vertex(f, 0),
                          floor_depth_shader.vertex(f, 1),
                          floor_depth_shader.vertex(f, 2) };
        rasterize(clip, floor_depth_shader, shadowbuffer, shadow_zbuffer);
    }

    DepthShader head_depth_shader(model);
    for (int f=0; f<model.nfaces(); f++) {
        Triangle clip = { head_depth_shader.vertex(f, 0),
                          head_depth_shader.vertex(f, 1),
                          head_depth_shader.vertex(f, 2) };
        rasterize(clip, head_depth_shader, shadowbuffer, shadow_zbuffer);
    }

    // Pass 2: regular camera render.
    lookat(eye, center, up);                                   // build the ModelView   matrix
    init_perspective(norm(eye-center));                        // build the Perspective matrix
    init_viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport    matrix
    init_zbuffer(width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB);

    SmoothShader floor_shader(light, floor_model, shadow_modelview, shadow_perspective, shadow_viewport, shadow_zbuffer, shadoww, shadowh);
    for (int f=0; f<floor_model.nfaces(); f++) {        // iterate through all facets
        Triangle clip = { floor_shader.vertex(f, 0),          // assemble the primitive
                          floor_shader.vertex(f, 1),
                          floor_shader.vertex(f, 2) };
        rasterize(clip, floor_shader, framebuffer, zbuffer);           // rasterize the primitive
    }

    SmoothShader head_shader(light, model, shadow_modelview, shadow_perspective, shadow_viewport, shadow_zbuffer, shadoww, shadowh);
    for (int f=0; f<model.nfaces(); f++) {              // iterate through all facets
        Triangle clip = { head_shader.vertex(f, 0),          // assemble the primitive
                          head_shader.vertex(f, 1),
                          head_shader.vertex(f, 2) };
        rasterize(clip, head_shader, framebuffer, zbuffer);           // rasterize the primitive
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
