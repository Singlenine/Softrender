#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fstream>

#pragma pack(push,1)
struct TGA_Header {
    char idlength;//ID字段长度
    char colormaptype;//颜色表类型
    char datatypecode;//数据类型码
    short colormaporigin;//颜色表起点
    short colormaplength;//颜色表长度
    char colormapdepth;//颜色表深度
    short x_origin;//X原点
    short y_origin;//Y原点
    short width;//宽度
    short height;//高度
    char  bitsperpixel;//每像素位数
    char  imagedescriptor;//图像描述符
};
#pragma pack(pop)

struct TGAColor {
    unsigned char bgra[4];
    unsigned char bytespp;

    TGAColor() : bgra(), bytespp(1) {
        for (int i=0; i<4; i++) bgra[i] = 0;
    }

    TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A=255) : bgra(), bytespp(4) {
        bgra[0] = B;
        bgra[1] = G;
        bgra[2] = R;
        bgra[3] = A;
    }

    TGAColor(unsigned char v) : bgra(), bytespp(1) {
        for (int i=0; i<4; i++) bgra[i] = 0;
        bgra[0] = v;
    }


    TGAColor(const unsigned char *p, unsigned char bpp) : bgra(), bytespp(bpp) {
        for (int i=0; i<(int)bpp; i++) {
            bgra[i] = p[i];
        }
        for (int i=bpp; i<4; i++) {
            bgra[i] = 0;
        }
    }

    unsigned char& operator[](const int i) { return bgra[i]; }

    TGAColor operator *(float intensity) const {
        TGAColor res = *this;
        intensity = (intensity>1.f?1.f:(intensity<0.f?0.f:intensity));
        for (int i=0; i<4; i++) res.bgra[i] = bgra[i]*intensity;
        return res;
    }
};

class TGAImage {
protected:
    unsigned char* data;
    int width;
    int height;
    int bytespp;

    bool   load_rle_data(std::ifstream &in);
    bool unload_rle_data(std::ofstream &out);
public:
    enum Format {
        GRAYSCALE=1, RGB=3, RGBA=4
    };

    TGAImage();
    TGAImage(int w, int h, int bpp);
    TGAImage(const TGAImage &img);
    bool read_tga_file(const char *filename);
    bool write_tga_file(const char *filename, bool rle=true);
    bool flip_horizontally();
    bool flip_vertically();
    bool scale(int w, int h);
    TGAColor get(int x, int y);
    bool set(int x, int y, TGAColor &c);
    bool set(int x, int y, const TGAColor &c);
    ~TGAImage();
    TGAImage & operator =(const TGAImage &img);
    int get_width();
    int get_height();
    int get_bytespp();
    unsigned char *buffer();
    void clear();
};

#endif //__IMAGE_H__