#ifndef DEDICATED_SERVER

#include "TileGraphicGL.hpp"
#include "ScopeAllocator.hpp"

TileGraphicGL::TileGraphicGL(int width, int height, bool keep_pictures)
    : TileGraphic(width, height, keep_pictures), sz(0) { }

TileGraphicGL::~TileGraphicGL() {
    for (Textures::iterator it = textures.begin(); it != textures.end(); it++) {
        GLuint tex = *it;
        glDeleteTextures(1, &tex);
    }

    for (Pictures::iterator it = pictures.begin(); it != pictures.end(); it++) {
        const PictureData& pd = *it;
        delete[] pd.pic;
    }
}

GLuint TileGraphicGL::get_texture() {
    return textures[current_index];
}

GLuint TileGraphicGL::get_texture(int index) {
    return textures[index % sz];
}

unsigned char *TileGraphicGL::get_picture_array(int index) {
    const PictureData& pd = pictures[index];

    return pd.pic;
}

int TileGraphicGL::get_bytes_per_pixel(int index) {
    const PictureData& pd = pictures[index];

    return pd.bytes_per_pixel;
}

void TileGraphicGL::add_tile(int bytes_per_pixel, const void *pic, bool desc, bool linear) {
    GLuint tex;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    if (linear) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, bytes_per_pixel, width, height, 0,
        (bytes_per_pixel == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, pic);

    if (desc) {
        textures.push_front(tex);
    } else {
        textures.push_back(tex);
    }
    sz = textures.size();
}

void TileGraphicGL::punch_out_tile(PNG& png, int tilex, int tiley, bool desc, bool linear) {
    int bytes_per_pixel = (png.ColorFormatRGBA ? 4 : 3);
    int row_len = png.get_width() * bytes_per_pixel;

    /* copy rect into a new 'temporary' picture array */
    unsigned char *buffer = new unsigned char[width * height * bytes_per_pixel];
    unsigned char *p = buffer;
    for (int y = 0; y < height; y++) {
        unsigned char *src = png.get_pic() + ((y + tiley) * row_len) + (tilex * bytes_per_pixel);
        for (int x = 0; x < width; x++) {
            for (int px = 0; px < bytes_per_pixel; px++) {
                *p++ = *src++;
            }
        }
    }

    /* now generate an OpenGL texture */
    add_tile(bytes_per_pixel, buffer, desc, linear);

    /* keep pictures for lightmap pixel-precise calculation */
    if (keep_pictures) {
        if (desc) {
            pictures.push_front(PictureData(bytes_per_pixel, buffer));
        } else {
            pictures.push_back(PictureData(bytes_per_pixel, buffer));
        }
    } else {
        delete[] buffer;
    }
}

bool TileGraphicGL::punch_out_lightmap(PNG& png, int tilex, int tiley) {
    int bytes_per_pixel = (png.ColorFormatRGBA ? 4 : 3);
    int row_len = png.get_width() * bytes_per_pixel;

    /* copy rect into a new temporary picture array */
    bool empty_lightmap = true;
    ScopeArrayAllocator<unsigned char> scope_buffer(width * height * bytes_per_pixel);
    unsigned char *dest = *scope_buffer;
    unsigned char *p = dest;
    for (int y = 0; y < height; y++) {
        unsigned char *src = png.get_pic() + ((y + tiley) * row_len) + (tilex * bytes_per_pixel);
        for (int x = 0; x < width; x++) {
            if (src[3] != 0) {
                empty_lightmap = false;
            }
            for (int px = 0; px < bytes_per_pixel; px++) {
                *p++ = *src++;
            }
        }
    }

    /* now generate an OpenGL texture */
    if (!empty_lightmap) {
        add_tile(bytes_per_pixel, dest, false, false);
    }

    return empty_lightmap;
}

size_t TileGraphicGL::get_tile_count() {
    return sz;
}

#endif
